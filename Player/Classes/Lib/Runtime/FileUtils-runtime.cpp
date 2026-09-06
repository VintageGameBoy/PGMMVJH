#include "Lib/Macros.h"
#ifdef USE_RUNTIME
#include "platform/CCPlatformConfig.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "platform/win32/CCFileUtils-win32.h"
#endif
#include "platform/CCCommon.h"
#include "FileUtils-runtime.h"
#include "base/base64.h"
// twofish.c 是 C 文件，头文件没有 extern "C" 保护，这里手动包裹以正确链接。
extern "C" {
#include "External/Twofish/twofish.h"
}

using namespace std;

NS_CC_BEGIN

#define CC_MAX_PATH  512

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
//static FileUtilsWin32 *s_platformFileUtils = nullptr;
#endif
static char s_key[16 + 1];

// PGMMV 官方加密的固定 IV（所有解密共用）。
static const unsigned char s_pgmmIv[16] = {
	0xA0, 0x47, 0xE9, 0x3D, 0x23, 0x0A, 0x4C, 0x62,
	0xA7, 0x44, 0xB1, 0xA4, 0xEE, 0x85, 0x7F, 0xBA
};

// 将字节序列左旋转 n 字节。
static void rolBytes(unsigned char *buf, int len, int n)
{
	if (len <= 0 || n <= 0) {
		return;
	}
	n %= len;
	unsigned char tmp[16];
	memcpy(tmp, buf, len);
	memcpy(buf, tmp + n, len - n);
	memcpy(buf + (len - n), tmp, n);
}

// 将字节序列右旋转 n 字节。
static void rorBytes(unsigned char *buf, int len, int n)
{
	if (len <= 0 || n <= 0) {
		return;
	}
	n %= len;
	unsigned char tmp[16];
	memcpy(tmp, buf, len);
	memcpy(buf, tmp + (len - n), n);
	memcpy(buf + n, tmp, len - n);
}

// Weakfish 解密：短密钥（<=8 字节）会使 Twofish 的密钥展开崩坏，
// 退化为与密钥值无关的固定字节旋转。
static void weakfishDecryptBlock(unsigned char *block)
{
	rorBytes(block, 4, 1);
	rolBytes(block + 4, 4, 1);
	rorBytes(block + 8, 4, 1);
	rolBytes(block + 12, 4, 1);
	rorBytes(block, 16, 8);
}

// CBC 解密。blockDecrypt 是解密单个 16 字节块的函数。
static void cbcDecrypt(unsigned char *data, int len, void (*blockDecrypt)(unsigned char *))
{
	unsigned char prev[16];
	unsigned char cur[16];
	memcpy(prev, s_pgmmIv, 16);
	for (int off = 0; off + 16 <= len; off += 16) {
		memcpy(cur, data + off, 16);
		blockDecrypt(data + off);
		for (int i = 0; i < 16; i++) {
			data[off + i] ^= prev[i];
		}
		memcpy(prev, cur, 16);
	}
}

// 长密钥（>8 字节）用的子密钥派生。
// 将明文长度的小端表示 XOR 到密钥开头（结果中的 0 替换为 1），
// 再拼接密钥剩余字节。
static void deriveSubkey(const unsigned char *key, int keyLen, unsigned long long ptLen, unsigned char *out, int *outLen)
{
	unsigned char ptl[8];
	memset(ptl, 0, 8);
	int ptlLen = 0;
	unsigned long long v = ptLen;
	while (v > 0) {
		ptl[ptlLen++] = (unsigned char)(v & 0xff);
		v >>= 8;
	}
	if (ptlLen == 0) {
		ptlLen = 1;
	}

	int xorLen = ptlLen < keyLen ? ptlLen : keyLen;
	for (int i = 0; i < xorLen; i++) {
		unsigned char b = ptl[i] ^ key[i];
		out[i] = (b == 0) ? 1 : b;
	}
	if (keyLen > xorLen) {
		memcpy(out + xorLen, key + xorLen, keyLen - xorLen);
	}
	*outLen = keyLen;
}

// The root path of resources, the character encoding is UTF-8.
// UTF-8 is the only encoding supported by cocos2d-x API.
static std::string s_resourcePath = "";

#if 0
FileUtils* FileUtils::getInstance()
{
    if (s_sharedFileUtils == nullptr)
    {
        s_sharedFileUtils = new FileUtilsWin32();
        if(!s_sharedFileUtils->init())
        {
          delete s_sharedFileUtils;
          s_sharedFileUtils = nullptr;
          CCLOG("ERROR: Could not init CCFileUtilsWin32");
        }
    }
    return s_sharedFileUtils;
}
#endif

FileUtilsRuntime::FileUtilsRuntime()
{
}

void FileUtilsRuntime::createAndSet()
{
	auto fileUtil = new FileUtilsRuntime();
	fileUtil->init();
	FileUtils::setDelegate(fileUtil);
}

void FileUtilsRuntime::setKey(const std::string &key)
{
	memcpy(s_key, key.c_str(), key.size() + 1);
}

const char *FileUtilsRuntime::key()
{
	return s_key;
}

void FileUtilsRuntime::setEncryptedKey(const std::string &base64Key)
{
	// Base64 解码
	unsigned char *decoded = nullptr;
	int decodedLen = base64Decode((const unsigned char *)base64Key.c_str(), (unsigned int)base64Key.size(), &decoded);
	if (decoded == nullptr || decodedLen <= 0) {
		if (decoded != nullptr) {
			free(decoded);
		}
		return;
	}

	// 用 Weakfish + CBC 解密（密钥是 "key" 这个短字符串，退化为 Weakfish）
	unsigned char *buf = (unsigned char *)malloc(decodedLen);
	memcpy(buf, decoded, decodedLen);
	free(decoded);

	// 按 16 字节块解密
	int blocks = decodedLen / 16;
	if (blocks * 16 == decodedLen) {
		unsigned char prev[16];
		unsigned char cur[16];
		memcpy(prev, s_pgmmIv, 16);
		for (int b = 0; b < blocks; b++) {
			memcpy(cur, buf + b * 16, 16);
			weakfishDecryptBlock(buf + b * 16);
			for (int i = 0; i < 16; i++) {
				buf[b * 16 + i] ^= prev[i];
			}
			memcpy(prev, cur, 16);
		}
	}

	// 去掉末尾的 \0 得到实际密钥
	int keyLen = decodedLen;
	while (keyLen > 0 && buf[keyLen - 1] == 0) {
		keyLen--;
	}

	if (keyLen > 0) {
		memset(s_key, 0, sizeof(s_key));
		memcpy(s_key, buf, keyLen < 16 ? keyLen : 16);
		s_key[16] = '\0';
	}
	free(buf);
}

#if 0
bool FileUtilsRuntime::init()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	//s_platformFileUtils = dynamic_cast<FileUtilsWin32 *>(FileUtils::getInstance());
#endif
    s_sharedFileUtils = this;
}

std::string FileUtilsRuntime::getWritablePath() const
{
    s_platformFileUtils->getWritablePath();
}

bool FileUtilsRuntime::isAbsolutePath(const std::string& strPath) const
{
    return s_platformFileUtils->isAbsolutePath(strPath);
}

std::string FileUtilsRuntime::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return s_platformFileUtils->getSuitableFOpen(filenameUtf8);
}

long FileUtilsRuntime::getFileSize(const std::string &filepath)
{
    return s_platformFileUtils->getFileSize(filepath);
}

#ifdef USE_AGTK//sakihama-h, 2016.11.15
std::string FileUtilsRuntime::getApplicationPath()
{
    return s_platformFileUtils->getApplicationPath();
}

std::vector<std::string> FileUtilsRuntime::getDirContents(std::string dirname)
{
    return s_platformFileUtils->getDirContents(dirname);
}

#endif


bool FileUtilsRuntime::isFileExistInternal(const std::string& strFilePath) const
{
    return s_platformFileUtils->isFileExistInternal(strFilePath);
}

bool FileUtilsRuntime::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    return s_platformFileUtils->renameFile(path, oldname, name);
}

bool FileUtilsRuntime::renameFile(const std::string &oldfullpath, const std::string &newfullpath)
{
    return s_platformFileUtils->renameFile(oldfullpath, newfullpath);
}

bool FileUtilsRuntime::isDirectoryExistInternal(const std::string& dirPath) const
{
    return s_platformFileUtils->isDirectoryExistInternal(dirPath);
}

bool FileUtilsRuntime::removeFile(const std::string &filepath)
{
    return s_platformFileUtils->removeFile(filepath);
}

bool FileUtilsRuntime::createDirectory(const std::string& dirPath)
{
    return s_platformFileUtils->createDirectory(dirPath);
}

bool FileUtilsRuntime::removeDirectory(const std::string& dirPath)
{
    return s_platformFileUtils->removeDirectory(dirPath);
}
#endif

FileUtils::Status FileUtilsRuntime::getContents(const std::string& filename, ResizableBuffer* buffer) const
{
	if (strlen(s_key) == 0) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		return FileUtilsWin32::getContents(filename, buffer);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	}
	Data d;
	ResizableBufferAdapter<Data> buf(&d);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	auto status = FileUtilsWin32::getContents(filename, &buf);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
    if(status != FileUtils::Status::OK){
		buffer->resize(d.getSize());
		memcpy(buffer->buffer(), d.getBytes(), d.getSize());
		d.clear();
        return status;
    }

	// 解密加密文件（"enc" 头）
	unsigned char *bytes = d.getBytes();
	ssize_t size = d.getSize();
	if (size >= 4 && memcmp(bytes, "enc", 3) == 0) {
		int padLen = bytes[3];
		ssize_t ptLen = size - 4 - padLen;
		unsigned char *cipher = bytes + 4;
		ssize_t cipherLen = size - 4;

		int keyLen = (int)strlen(s_key);
		if (keyLen <= 8) {
			// 短密钥：用 Weakfish（固定旋转）解密
			cbcDecrypt(cipher, (int)cipherLen, weakfishDecryptBlock);
		}
		else {
			// 长密钥：用 Twofish + 子密钥派生解密
			unsigned char subkey[32];
			int subkeyLen = 0;
			deriveSubkey((const unsigned char *)s_key, keyLen, (unsigned long long)ptLen, subkey, &subkeyLen);

			Twofish_initialise();
			Twofish_key xkey;
			Twofish_prepare_key(subkey, subkeyLen, &xkey);

			unsigned char prev[16];
			unsigned char cur[16];
			memcpy(prev, s_pgmmIv, 16);
			for (ssize_t off = 0; off + 16 <= cipherLen; off += 16) {
				memcpy(cur, cipher + off, 16);
				unsigned char out[16];
				Twofish_decrypt(&xkey, cipher + off, out);
				for (int i = 0; i < 16; i++) {
					cipher[off + i] = out[i] ^ prev[i];
				}
				memcpy(prev, cur, 16);
			}
		}

		// 截断到明文长度后写回
		buffer->resize((size_t)ptLen);
		memcpy(buffer->buffer(), cipher, (size_t)ptLen);
		d.clear();
		return status;
	}

	buffer->resize(d.getSize());
	memcpy(buffer->buffer(), d.getBytes(), d.getSize());
	d.clear();
	return status;
}

#if 0
std::string FileUtilsRuntime::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    return s_platformFileUtils->getPathForFilename(filename, resolutionDirectory, searchPath);
}

std::string FileUtilsRuntime::getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const
{
    return s_platformFileUtils->getFullPathForDirectoryAndFilename(directory, filename);
}
#endif

NS_CC_END

#endif