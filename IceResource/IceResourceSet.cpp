#include "IceResourceSet.h"
#include "StdAfx.h"

#include "NullProgressMonitor.h"
#include "PackResource.h"
#include "Utility.h"
#include "Win32File.h"
#include "types.h"

#include "clCRC.h"
#include "clICE.h"

//////////////////////////////////////////////////////////////////////////
shared_ptr<vector<char>> GetNameChars(LPCSTR lpszName) {
	int nLen = lstrlenA(lpszName) + 1; // 结束符
	if (nLen <= 0x60 - 1) {
		shared_ptr<vector<char>> spBuffer;
		char type;
		if (nLen < 0x40 - 1) {
			type = nLen / 0x10;
			spBuffer.reset(new vector<char>((type + 1) * 0x10));
		}
		else {
			type = 4;
			spBuffer.reset(new vector<char>(0x60));
		}

		*spBuffer->begin() = type;

		lstrcpyA(&(*spBuffer->begin()) + 1, lpszName);

		return spBuffer;
	}
	else {
		DWORD dwLen = (nLen + 5) % 0x10;
		dwLen = (dwLen == 0) ? nLen : (nLen - dwLen + 0x10);

		shared_ptr<vector<char>> spBuffer(new vector<char>(5 + dwLen));

		memset(&(*spBuffer->begin()), 0, dwLen + 5);
		*spBuffer->begin() = 5;
		memcpy(&(*spBuffer->begin()) + 1, &dwLen, 2);
		lstrcpyA(&(*spBuffer->begin()) + 5, lpszName);
		return spBuffer;
	}
}
//////////////////////////////////////////////////////////////////////////
IResourceSet *IResourceSet::CreateResourceSetFromFile(LPCTSTR lpszFile) {
	CPackResourceSet *pResource = new CPackResourceSet();
	pResource->Open(lpszFile);
	return pResource;
}

bool IResourceSet::PackResources(IResource **resources, size_t size,
	size_t version, LPCTSTR lpszPackFile,
	IProgressMonitor *pMonitor) {

	clCRC crc;

	NullProgressMonitor nullProgressMonitor;
	if (pMonitor == NULL) {
		pMonitor = &nullProgressMonitor;
	}

	pMonitor->BeginWork(MAKEINTRESOURCE(IDS_MAKE_PACK_FILE), size + 3);

	pMonitor->SetSubTaskName(MAKEINTRESOURCE(IDS_INIT_PACK_HEADER));

	CWin32File outFile(lpszPackFile, true);

	// Generate group & compression
	stGroup g;
	memset(&g, 0, sizeof(g));
	unsigned char *group1 = nullptr, *group2 = nullptr;

	pMonitor->Progress(1);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	{

		for (stFile *file = _fileFirst; file; file = file->next) {
			if (file->groupIndex == 0 && file->header) {
				g.group1.originalSize += file->header->fileSize;
			}
		}
		unsigned char *buf = new unsigned char[g.group1.originalSize], *p = buf;
		if (!buf) {
			fclose(fp);
			return false;
		}
		for (stFile *file = _fileFirst; file; file = file->next) {
			if (file->groupIndex == 0 && file->header) {
				memcpy(p, file->header, file->header->fileSize);
				p += file->header->fileSize;
				g.group1.fileCount++;
			}
		}

		if (mode_compress) {
			printf("Compressing Gruop1...\n");
			group1 = Compress(buf, g.group1.originalSize, &g.group1.dataSize);
		}
		if (group1 && g.group1.originalSize > g.group1.dataSize) {
			delete[] buf;
		}
		else {
			if (group1) {
				delete[] group1;
			}
			group1 = buf;
			g.group1.dataSize = 0;
		}
		g.group1Size =
			g.group1.dataSize ? g.group1.dataSize : g.group1.originalSize;
	}

	// Prosessing Gruop2
	pMonitor->Progress(2);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	{
		for (stFile *file = _fileFirst; file; file = file->next)
			if (file->groupIndex == 1 && file->header)
				g.group2.originalSize += file->header->fileSize;
		unsigned char *buf = new unsigned char[g.group2.originalSize], *p = buf;
		if (!buf) {
			if (group1)
				delete[] group1;
			fclose(fp);
			return false;
		}
		for (stFile *file = _fileFirst; file; file = file->next)
			if (file->groupIndex == 1 && file->header) {
				memcpy(p, file->header, file->header->fileSize);
				p += file->header->fileSize;
				g.group2.fileCount++;
			}
		if (mode_compress) {
			printf("Compressing Gruop2...\n");
			group2 = Compress(buf, g.group2.originalSize, &g.group2.dataSize);
		}
		if (group2 && g.group2.originalSize > g.group2.dataSize) {
			delete[] buf;
		}
		else {
			if (group2) {
				delete[] group2;
			}
			group2 = buf;
			g.group2.dataSize = 0;
		}
		g.group2Size =
			g.group2.dataSize ? g.group2.dataSize : g.group2.originalSize;
	}

	// Generating header
	pMonitor->Progress(3);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	// Generate header info
	stHeader h;
	memset(&h, 0, sizeof(h));
	h.signature = SIGNATURE;
	h.version = ICE_VERSION;
	h.r1 = 0x80;
	stInfo i;
	memset(&i, 0, sizeof(i));
	i.r1 = 0xFF;
	i.r2 = 1;
	unsigned char t[0x100];
	memset(t, 0, sizeof(t));
	if (mode_crc)
		memcpy(t, _t, sizeof(_t)); // Maintain random table
	i.crc32 =
		crc.GetCRC32(group2, g.group2Size, crc.GetCRC32(group1, g.group1Size));
	i.filesize = sizeof(h) + sizeof(i) + sizeof(t) + sizeof(g) + g.group1Size +
		g.group2Size;

	// Generate key group1, group2
	pMonitor->Progress(4);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	static int list13[] = { 13, 17, 4, 7, 5, 14 };
	static int list17[] = { 17, 25, 15, 10, 28, 8 };
	unsigned int key = GetKey3(
		GetKey2(GetKey1(i.filesize, t, h.version), t, h.version), t, h.version);
	unsigned int groupHeadersKey =
		rol(key, h.version <= 9 ? list13[h.version - 4] : 0);
	unsigned int group1BlowFishKey1 = key;
	unsigned int group1BlowFishKey2 = GetKey2(key, t, h.version);
	unsigned int group2BlowFishKey1 =
		rol(group1BlowFishKey1, h.version <= 9 ? list17[h.version - 4] : 0);
	unsigned int group2BlowFishKey2 =
		rol(group1BlowFishKey2, h.version <= 9 ? list17[h.version - 4] : 0);


	// Encryption
	pMonitor->Progress(5);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	Encrypt(group1, g.group1Size, h.version, group1BlowFishKey1,
		group1BlowFishKey2);
	g.group1.crc32 = crc.GetCRC32(group1, g.group1Size);
	Encrypt(group2, g.group2Size, h.version, group2BlowFishKey1,
		group2BlowFishKey2);
	g.group2.crc32 = crc.GetCRC32(group2, g.group2Size);

	// Maintain CRC (temporary)
	pMonitor->Progress(6);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	if (mode_crc) {
		i.crc32 = _crc32[0];
		g.group1.crc32 = _crc32[1];
		g.group2.crc32 = _crc32[2];
	}

	// Write
	pMonitor->Progress(7);
	if (pMonitor->IsCanceled()) {
		return false;
	}
	outFile.Write(&h, 1);
	outFile.Write(&i, 1);
	outFile.Write(&t, 1);

	stGroup group;
	memcpy(&group, &g, sizeof(group));

	clBlowFish bf(&groupHeadersKey, sizeof(groupHeadersKey), true);
	bf.Encrypt(&group, sizeof(group));
	outFile.Write(&group, 1);
	if (group1) {
		outFile.Write(&group, 1);
		delete[] group1;
	}
	if (group2) {
		outFile.Write(&group, 1);
		delete[] group2;
	}

	outFile.Close();
	return true;
/*
	PACKAGE_HEADER header;
	memcpy(header.signature, "PACK\002\001\0\0", 8);
	header.d1 = 1;
	header.sum = size;

	FILETIME ft;
	SYSTEMTIME st;

	GetSystemTime(&st);             // gets current time
	SystemTimeToFileTime(&st, &ft); // converts to file time format
	header.ft1 = ft;
	header.ft2 = ft;

	memset(header.path, 0, 480);
	lstrcpyA(header.path, "data\\");

	PACKAGE_LIST_HEADER listHeader;
	listHeader.sum = size;
	// 预先设定一个文件列表空间
	listHeader.list_header_size = 0;
	listHeader.blank_size = 0;
	listHeader.data_section_size = 0;
	memset(listHeader.zero, 0, 16);

	vector<shared_ptr<vector<char>>> array_item_name_chars;
	for (size_t i = 0; i < size; i++) {
		shared_ptr<vector<char>> namechars =
			GetNameChars(ConvertToANSI(resources[i]->GetName()));

		array_item_name_chars.push_back(namechars);

		listHeader.list_header_size += (namechars->size() + sizeof(ITEM_INFO));
	}

	pMonitor->Progress(1);
	if (pMonitor->IsCanceled()) {
		return false;
	}

	CWin32File outFile(lpszPackFile, true);
	outFile.Seek(sizeof(PACKAGE_HEADER) + sizeof(PACKAGE_LIST_HEADER) +
		listHeader.list_header_size,
		FILE_BEGIN);

	vector<shared_ptr<ITEM_INFO>> array_info;
	for (size_t i = 0; i < size; i++) {
		pMonitor->SetSubTaskName(resources[i]->GetName());

		size_t compressedSize = resources[i]->GetCompressedSize();
		vector<char> compressedContent(compressedSize);
		compressedSize = resources[i]->GetCompressedContent(&compressedContent[0],
			compressedSize, level);
		compressedContent.resize(compressedSize);

		// 写入文件需要加密
		CUtility::Encrypt(&compressedContent[0], compressedSize, version);
		outFile.Write(&compressedContent[0], compressedSize);

		shared_ptr<ITEM_INFO> spInfo(new ITEM_INFO);
		spInfo->compress_size = resources[i]->GetCompressedSize();
		spInfo->decompress_size = resources[i]->GetDecompressedSize();
		spInfo->is_compressed = 1;
		spInfo->offset = listHeader.data_section_size;
		spInfo->seed = version;
		spInfo->ft[0] = spInfo->ft[1] = resources[i]->GetCreationTime();
		spInfo->ft[2] = spInfo->ft[3] = resources[i]->GetLastAccessTime();
		spInfo->ft[4] = resources[i]->GetLastWriteTime();

		array_info.push_back(spInfo);

		listHeader.data_section_size += compressedSize;

		pMonitor->Progress(1);
		if (pMonitor->IsCanceled()) {
			return false;
		}
	}

	pMonitor->SetSubTaskName(MAKEINTRESOURCE(IDS_WRITE_PACK_HEADER));
	outFile.Seek(0, FILE_BEGIN);
	outFile.Write(&header, sizeof(PACKAGE_HEADER));
	outFile.Write(&listHeader, sizeof(PACKAGE_LIST_HEADER));

	pMonitor->Progress(1);
	if (pMonitor->IsCanceled()) {
		return false;
	}

	pMonitor->SetSubTaskName(MAKEINTRESOURCE(IDS_WRITE_FILE_LIST));
	for (size_t i = 0; i < size; i++) {
		outFile.Write(&(*array_item_name_chars[i]->begin()),
			array_item_name_chars[i]->size());
		outFile.Write(array_info[i].get(), sizeof(ITEM_INFO));
	}

	pMonitor->Progress(1);
	if (pMonitor->IsCanceled()) {
		return false;
	}

	pMonitor->Done();

	return true;
*/
}

//////////////////////////////////////////////////////////////////////////

CPackResourceSet::CPackResourceSet(void) {}

CPackResourceSet::~CPackResourceSet(void) {}

bool CPackResourceSet::Open(LPCTSTR lpszPackFile) {
	shared_ptr<CWin32File> spFile(new CWin32File(lpszPackFile));
	if (!spFile->IsOK()) {
		// 打开文件错误
		return false;
	}
	// release
	Release();
rew
	// Open file
	FILE *fp;
	if (fopen_s(&fp, path, "rb"))
		return false;

	// Header check
	stHeader h;
	fread(&h, sizeof(h), 1, fp);
	if (!CheckFile(&h, sizeof(h))) {
		fclose(fp);
		return false;
	}

	// Process by version
	stGroup g;
	_version = h.version;
	switch (h.version) {
	case 3: {

		// Get information
		stInfo i;
		fread(&g, sizeof(g), 1, fp);
		fread(&i, sizeof(i), 1, fp);
		fseek(fp, 0x30, SEEK_CUR);
		_crc32[0] = i.crc32;
		_crc32[1] = g.group1.crc32;
		_crc32[2] = g.group2.crc32;
		memset(_t, 0, sizeof(_t));

		// generate key
		unsigned int key = g.group1Size;
		if (key)
			key = bswap(key);
		else if (i.r2)
			key = GetKey(&g);

		// Group1
		if (g.group1.dataSize) {
			unsigned char *data = new unsigned char[g.group1.dataSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group1.dataSize, 1, fp);
			Decrypt(data, g.group1.dataSize, h.version, key, 0);
			_group1 = Uncompress(data, g.group1.dataSize, g.group1.originalSize);
			delete[] data;
		}
		else if (g.group1.originalSize) {
			unsigned char *data = new unsigned char[g.group1.originalSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group1.originalSize, 1, fp);
			Decrypt(data, g.group1.originalSize, h.version, key, 0);
			_group1 = data;
		}

		// Group2
		if (g.group2.dataSize) {
			unsigned char *data = new unsigned char[g.group2.dataSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group2.dataSize, 1, fp);
			Decrypt(data, g.group2.dataSize, h.version, key, 0);
			_group2 = Uncompress(data, g.group2.dataSize, g.group2.originalSize);
			delete[] data;
		}
		else if (g.group2.originalSize) {
			unsigned char *data = new unsigned char[g.group2.originalSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group2.originalSize, 1, fp);
			Decrypt(data, g.group2.originalSize, h.version, key, 0);
			_group2 = data;
		}

	} break;
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9: {

		// Get information
		stInfo i;
		unsigned char t[0x100];
		fread(&i, sizeof(i), 1, fp);
		if (h.version > 4)
			fseek(fp, 0x10, SEEK_CUR);
		fread(t, sizeof(t), 1, fp);
		fread(&g, sizeof(g), 1, fp);

		// generate key for group1, group2
		static int list13[] = { 13, 17, 4, 7, 5, 14 };
		static int list17[] = { 17, 25, 15, 10, 28, 8 };
		unsigned int key = GetKey3(
			GetKey2(GetKey1(i.filesize, t, h.version), t, h.version), t, h.version);
		unsigned int groupHeadersKey =
			rol(key, h.version <= 9 ? list13[h.version - 4] : 0);
		unsigned int group1BlowFishKey1 = key;
		unsigned int group1BlowFishKey2 = GetKey2(key, t, h.version);
		unsigned int group2BlowFishKey1 =
			rol(group1BlowFishKey1, h.version <= 9 ? list17[h.version - 4] : 0);
		unsigned int group2BlowFishKey2 =
			rol(group1BlowFishKey2, h.version <= 9 ? list17[h.version - 4] : 0);

		// Decrypt group information
		clBlowFish bf(&groupHeadersKey, sizeof(groupHeadersKey), true);
		bf.Decrypt(&g, sizeof(g));


		// Maintain CRC (temporary)
		_crc32[0] = i.crc32;
		_crc32[1] = g.group1.crc32;
		_crc32[2] = g.group2.crc32;
		memcpy(_t, t, sizeof(t));

		// Group1
		if (g.group1.dataSize) {
			unsigned char *data = new unsigned char[g.group1.dataSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group1.dataSize, 1, fp);
			Decrypt(data, g.group1.dataSize, h.version, group1BlowFishKey1,
				group1BlowFishKey2);
			_group1 = Uncompress(data, g.group1.dataSize, g.group1.originalSize);
			delete[] data;
		}
		else if (g.group1.originalSize) {
			unsigned char *data = new unsigned char[g.group1.originalSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group1.originalSize, 1, fp);
			Decrypt(data, g.group1.originalSize, h.version, group1BlowFishKey1,
				group1BlowFishKey2);
			_group1 = data;
		}

		// Group2
		if (g.group2.dataSize) {
			unsigned char *data = new unsigned char[g.group2.dataSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group2.dataSize, 1, fp);
			Decrypt(data, g.group2.dataSize, h.version, group2BlowFishKey1,
				group2BlowFishKey2);
			_group2 = Uncompress(data, g.group2.dataSize, g.group2.originalSize);
			delete[] data;
		}
		else if (g.group2.originalSize) {
			unsigned char *data = new unsigned char[g.group2.originalSize];
			if (!data) {
				fclose(fp);
				return false;
			}
			fread(data, g.group2.originalSize, 1, fp);
			Decrypt(data, g.group2.originalSize, h.version, group2BlowFishKey1,
				group2BlowFishKey2);
			_group2 = data;
		}
	} break;
	default:
		fclose(fp);
		return false;
	}

	// Close file
	fclose(fp);

	// Add list
	stFile::stHeader *file = (stFile::stHeader *)_group1;
	for (unsigned int i = 0; i < g.group1.fileCount && file;
		i++, file = (stFile::stHeader *)((char *)file + file->fileSize)) {
		stFile *f = new stFile;
		if (!f)
			return false;
		memset(f, 0, sizeof(*f));
		f->groupIndex = 0;
		f->header = file;
		if (_fileLast)
			_fileLast->next = f;
		else
			_fileFirst = f;
		_fileLast = f;
	}
	file = (stFile::stHeader *)_group2;
	for (unsigned int i = 0; i < g.group2.fileCount && file;
		i++, file = (stFile::stHeader *)((char *)file + file->fileSize)) {
		stFile *f = new stFile;
		if (!f)
			return false;
		memset(f, 0, sizeof(*f));
		f->groupIndex = 1;
		f->header = file;
		if (_fileLast)
			_fileLast->next = f;
		else
			_fileFirst = f;
		_fileLast = f;
	}

	return true;
}

int CPackResourceSet::FindResourceIndex(LPCTSTR lpszName) {
	return CUtility::BinaryFindResourceIndex(m_Resources, lpszName);
}

size_t CPackResourceSet::GetResourceCount() { return m_Resources.size(); }

IResource *CPackResourceSet::GetResource(size_t index) {
	return m_Resources.at(index).get();
}

void CPackResourceSet::Release() { delete this; }

static char *ConvertToANSI(const wchar_t *szUnicode) {
	LPSTR szAnsi;
	int len = ::WideCharToMultiByte(CP_THREAD_ACP, 0, szUnicode, -1, NULL, 0,
		NULL, NULL);
	szAnsi = (LPSTR)malloc(len + 1);
	memset(szAnsi, 0, len + 1);
	::WideCharToMultiByte(CP_THREAD_ACP, 0, szUnicode, -1, szAnsi, len, NULL,
		NULL);

	return szAnsi;
}

static wchar_t *ConvertToWide(const char *text) {
	wchar_t *szUnicode = NULL;
	if (!text || !text[0])
		return szUnicode;
	const int wlen = ::MultiByteToWideChar(CP_THREAD_ACP, 0, text, -1, NULL, 0);
	if (!wlen)
		return szUnicode;

	szUnicode = new wchar_t[wlen + 1];
	if (::MultiByteToWideChar(CP_THREAD_ACP, 0, text, -1, szUnicode, wlen))
		szUnicode[wlen] = L'\0';

	return szUnicode;
}
