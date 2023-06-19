#include <windows.h>
#include <time.h>
#include <iostream>
using namespace std;

//2020.8.9������£��޸�ͨ�����СBUG����BUG���м��ͼ����������������޸�
//2022.4.4������£����һ����룬�޸���ʼ��ַ��СBUG����BUG����С�����Ѳ�����Ҫ���ڴ�

/*
findMatchingCode() ����˵����
1) hProcess		Ҫ�򿪵Ľ��̾��
2) markCode		������,֧��ͨ�����**������: 55 8b ec ** 56 83 ec 20 ** ** 08 d9 ee
3) memBeginAddr		��ʼ������ַ
4) memEndAddr		����������ַ
5) retAddr[]		��¼�ҵ��ĵ�ַ,�����������ǰһ��Ҫ��0���� DWORD retAddr[32] = {0};  ���� DWORD *retAddr = new DWORD[32]();
6) deviation		�������ַ��Ŀ���ַ��ƫ�ƾ��룬�ϸ�����
7) isCall		�Ƿ�Ϊ��CALL����ת��ַ��true �� retAddr[] ���ص���CALL��ת�ĵ�ַ
8) isAll		�Ƿ�������з��ϵĵ�ַ��false�ҵ���һ�����ϵĵ�ַ��ͽ���������true����������ֱ��������ַ���ڽ�����ַ��memEndAddr��
return����ֵ		�ҵ��ĵ�ַ����
�Ѳ����ڴ�����Ǳ�������û��ѡ��
*/
DWORD findMatchingCode(HANDLE hProcess, string markCode, DWORD memBeginAddr, DWORD memEndAddr, DWORD retAddr[], int deviation, bool isCall, bool isAll = false);

DWORD findMatchingCode(HANDLE hProcess, string markCode, DWORD memBeginAddr, DWORD memEndAddr, DWORD retAddr[], int deviation, bool isCall, bool isAll)
{
	//----------------------����������----------------------//
	//ȥ�����пո�
	if (!markCode.empty())
	{
		int index = 0;
		while ((index = markCode.find(' ', index)) >= 0)
		{
			markCode.erase(index, 1);
		}
		index = 0;
		while (true)
		{
			//ɾ��ͷ��ͨ���
			index = markCode.find("**", index);
			if (index == 0) {
				markCode.erase(index, 2);
			}
			else {
				break;
			}
		}
	}

	//�����볤�Ȳ���Ϊ����
	if (markCode.length() % 2 != 0) return 0;

	//�����볤��
	int len = markCode.length() / 2;

	//Sunday�㷨ģ������ĳ���
	int nSundayLen = len;

	//��������ת����byte��
	BYTE* pMarkCode = new BYTE[len];
	BYTE* pWildcard = new BYTE[len];
	for (int i = 0; i < len; i++) {
		string tempStr = markCode.substr(i * 2, 2);
		if (tempStr == "**") {
			pWildcard[i] = 0xFF;
			if (nSundayLen == len) nSundayLen = i;	//��¼��һ��ͨ�����������������Խ����Ч��Խ��
		}
		else {
			pWildcard[i] = 0x00;
		}
		pMarkCode[i] = strtoul(tempStr.c_str(), 0, 16);
	}
	//--------------------------end-------------------------//

	//Sunday�㷨ģ�����鸳ֵ��+1��ֹ���������FFʱԽ��
	int aSunday[0xFF + 1] = { 0 };
	for (int i = 0; i < nSundayLen; i++) {
		aSunday[pMarkCode[i]] = i + 1;
	}

	//��ʼ��ַ
	const DWORD dwBeginAddr = memBeginAddr;
	//������ַ
	const DWORD dwEndAddr = memEndAddr;
	//��ǰ��ȡ���ڴ���ַ
	DWORD dwCurAddr = dwBeginAddr;
	//����ڴ����ݵĻ�����
	BYTE* pMemBuffer = NULL;
	//�������retAddr[]����ĳ��ȣ��ò�������ǰһ��Ҫ��0
	int nArrayLength = 0;
	for (int i = 0; ; i++) {
		if (*(retAddr + i) != 0) {
			nArrayLength = i;
			break;
		}
	}
	//ƫ����
	int nOffset;
	//�����±꣺�ڴ桢�����롢���ص�ַ
	int i = 0, j = 0, nCount = 0;

	//�ڴ���Ϣ
	MEMORY_BASIC_INFORMATION mbi;

	//��¼��ʼ����ʱ��
	clock_t nBeginTime = clock();

	//ɨ���ڴ�
	while (dwCurAddr < dwEndAddr)
	{
		//��ѯ��ַ�ռ����ڴ��ַ����Ϣ
		memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
		if (::VirtualQueryEx(hProcess, (LPCVOID)dwCurAddr, &mbi, sizeof(mbi)) == 0) {
			goto end;
		}

		//���һ����룬�޸ĵ�ǰ��ַ������ڴ�ҳ����ʼ��ַ���༭��2022.4.4��û�������и����Ѳ�����Ҫ���ڴ�
		dwCurAddr = (DWORD_PTR)mbi.BaseAddress;
		//-----------------------------------------------------//

		//�����ڴ�ռ�, �����ڴ��״̬�ͱ������Խ��й���
		//һ��ɨ�裨��д��ִ�У����ɣ��ٶȼ��죬ɨ�����Ļ��ڳ�����ӣ���д����һ����
		if (MEM_COMMIT == mbi.State &&			//�ѷ���������ڴ�
			//MEM_PRIVATE == mbi.Type ||		//˽���ڴ棬�����������̹���
			//MEM_IMAGE == mbi.Type &&
			//PAGE_READONLY == mbi.Protect ||	//ֻ��
			//PAGE_EXECUTE_READ == mbi.Protect ||	//����ִ��
			//PAGE_READWRITE == mbi.Protect ||	//��д
			PAGE_EXECUTE_READWRITE == mbi.Protect)	//��д��ִ��
		{
			//���붯̬�ڴ�
			if (pMemBuffer) {
				delete[] pMemBuffer;
				pMemBuffer = NULL;
			}
			pMemBuffer = new BYTE[mbi.RegionSize];
			//��ȡ�����ڴ�
			ReadProcessMemory(hProcess, (LPCVOID)dwCurAddr, pMemBuffer, mbi.RegionSize, 0);
			i = 0;
			j = 0;
			while (j < len)
			{
			nextAddr:
				if (pMemBuffer[i] == pMarkCode[j] || pWildcard[j] == 0xFF)
				{
					i++;
					j++;
				}
				else
				{
					nOffset = i - j + nSundayLen;
					//�ж�ƫ�����Ƿ���ڻ�����
					if (nOffset > mbi.RegionSize - len) break;
					//�ж� aSundayģ������ ����û�� �ڴ�ƫ�ƺ��ֵ��������ݣ�����+1
					if (aSunday[pMemBuffer[nOffset]])
					{
						i = nOffset - aSunday[pMemBuffer[nOffset]] + 1;
						j = 0;
					}
					else
					{
						i = nOffset + 1;
						j = 0;
					}
				}
			}

			if (j == len)
			{
				//�����ҵ���Ŀ���ַ��
				//�������ַ = ��ǰ�ڴ���ַ + iƫ�� - �����볤��
				//Ŀ���ַ = �������ַ + ƫ�ƾ���
				//CALL��E8����ת�ĵ�ַ = E8ָ������4���ֽڵ�ַ + ��һ��ָ���ַ(Ҳ����Ŀ���ַ + 5)
				retAddr[nCount] = dwCurAddr + i - len + deviation;
				if (isCall) {
					DWORD temp;
					memcpy(&temp, &pMemBuffer[i - len + deviation + 1], 4);
					retAddr[nCount] += 5;
					retAddr[nCount] += temp;
				}

				if (++nCount >= nArrayLength)
				{
					//����������±�Խ��ͽ�������
					goto end;
				}

				if (isAll) {
					i = i - len + 1;
					j = 0;
					goto nextAddr;
				}
				else {
					goto end;
				}
			}
			dwCurAddr += mbi.RegionSize; //ȡ��һ���ڴ��ַ
		}
		else
		{
			dwCurAddr += mbi.RegionSize;
		}
	}

end:
	//����������ʱ(ms)
	clock_t nEndTime = clock();
	int nUseTime = (nEndTime - nBeginTime);
	//�ͷ��ڴ�
	if (pMemBuffer) {
		delete[] pMemBuffer;
		pMemBuffer = NULL;
	}
	delete[] pMarkCode;
	pMarkCode = NULL;
	delete[] pWildcard;
	pWildcard = NULL;
	return nCount;
}