// �긴�� ����

// ����� �̿��� ����
// ���� ������ �������̽��� ��ӹ޴� Ŭ�������� �Ѵ�.

// ����Ŭ������ ���·� �ǰ�ü�� �����ϱ� ������ ������ ��ü�� ����
// ���� ������ �ƴϱ� ������ ���谡 ����������.

// ����
// 1. �� ���͸� ������ �� ���� ���� �ٸ� �������̽��� �ο��ϰ� �ൿ ����
// 2. �������̽��� ��ü�ϰ� �ൿ ����

// ���� ��
// 1. ����Ƽ �������� ������Ʈ�� ���ҽ����� ���� �ٲ㰡�� ������ �� �ִ� �Ͱ� ����ϴٰ� ����
// 2. ó���� ���� �������̵����� �������ϸ�(������) ���� �������� ���������, 
//    �ϳ��� Ŭ������ �������̽��� �ٲ㰡�� �����ϴ� �͵� ���� ����̶�� ����
//    (ex. Sword_1, Sword_2ó�� �������̵��ذ��鼭 Ŭ������ �ø��� �������, Sword��� Ŭ������ �������̽��� �ٲ㰡�� ����ϴ� ���� ȿ�����̶� ����)

#include<iostream>

using namespace std;

class CBase abstract
{
	int			m_iRefCnt = 0;

public:
	void	Set_AddRef() { ++m_iRefCnt; }
	void	Set_Release()
	{
		if (0 == m_iRefCnt)
		{
			Free();
			delete this;
			return;
		}
		else
			--m_iRefCnt;
	}

public:
	virtual void Free() = 0;
};

class CInterface abstract : public CBase
{ 
public:
	// ���� ������ ��ӹ޴� Ŭ�������� �Ѵ�
	virtual void Skill_0() = 0;
	virtual void Skill_1() = 0;
	virtual void Skill_2() = 0;

public:
	virtual void Free() {}
	
};

class CInterface_Testudo final : public CInterface
{
public:
	// ���� ����
	virtual void Skill_0() { cout << "������ ����" << endl; }
	virtual void Skill_1() { cout << "���� ���Ŵ�" << endl; }
	virtual void Skill_2() { cout << "������ �Ѵ�" << endl; }

public:
	virtual void Free()
	{
		CInterface::Free();

		cout << "�׽����� ���� �������̽� ����" << endl;
	};
};

class CInterface_Angkara final : public CInterface
{
public:
	// ���� ����
	virtual void Skill_0() { cout << "���̸� ã�´�" << endl; }
	virtual void Skill_1() { cout << "���Ÿ� �߰��Ѵ�" << endl; }
	virtual void Skill_2() { cout << "���Ÿ� ���� �޷�����" << endl; }

public:
	virtual void Free()
	{
		CInterface::Free();

		cout << "��ī�� ���� �������̽� ����" << endl;
	};
};

class CMonster abstract : public CBase
{
protected:
	CInterface*		m_pInsterface = nullptr;
	char*			m_szName = nullptr;
	
protected:
	CMonster() {}
	CMonster(const char* _pszName, CInterface* _pInterface) 
	{
		m_szName = new char[strlen(_pszName) + 1];
		strcpy(m_szName, _pszName);

		m_pInsterface = _pInterface;

		if (nullptr != m_pInsterface)
			m_pInsterface->Set_AddRef();
	}
	CMonster(const CMonster& _rhs)
	{
		if (nullptr != m_szName)
			delete[] m_szName;

		m_szName = new char[strlen(_rhs.m_szName) + 1];
		strcpy(m_szName, _rhs.m_szName);

		m_pInsterface = _rhs.m_pInsterface;
	}
	CMonster(CMonster&& _rhs)
	{
		m_szName = _rhs.m_szName;
		m_pInsterface = _rhs.m_pInsterface;

		_rhs.m_szName = nullptr;
		_rhs.m_pInsterface = nullptr;
	}
	~CMonster() = default;

public:
	CInterface*	Get_Insterface() { return m_pInsterface; }

public:
	void	Set_Insterface(CInterface* _pInsterface) 
	{ 
		if(nullptr != m_pInsterface)
			m_pInsterface->Set_Release();

		m_pInsterface = _pInsterface; 

		if(nullptr != m_pInsterface)
			m_pInsterface->Set_AddRef();
	}

public:
	// �߻�ȭ�� ����� ȣ�� -> �������̽��� �� ��ü�� ����� ȣ���� �� ����
	virtual void Skill_0() 
	{
		if(nullptr != m_pInsterface)
			m_pInsterface->Skill_0(); 
	}
	virtual void Skill_1() 
	{
		if (nullptr != m_pInsterface)
			m_pInsterface->Skill_1(); 
	}
	virtual void Skill_2() 
	{
		if (nullptr != m_pInsterface)
			m_pInsterface->Skill_2(); 
	}

	virtual void Action() = 0;

public:
	// �������̽��� ���Ϳ� ���� �����ϴ� �۾��� �ϱ� ������
	// RefCnt�� ���ҽ�Ű�� �۾��� �ʿ�
	void Release_Insterface() 
	{
		if (nullptr != m_pInsterface)
			m_pInsterface->Set_Release();
	}
	virtual void Free()
	{
		Release_Insterface();

		delete[] m_szName;

		if(nullptr != m_pInsterface)
			m_pInsterface->Set_Release();
	}
};

class CTestudo final : public CMonster
{
public:
	CTestudo() {}
	CTestudo(const char* _pszName, CInterface* _pInterface) : CMonster(_pszName, _pInterface) {}
	CTestudo(const CTestudo& _rhs) : CMonster(_rhs) {}
	CTestudo(CTestudo&& _rhs) : CMonster(_rhs) {}
	~CTestudo() = default;

public:
	// �������̽��� ���� ����� �޶���
	void Action()
	{
		if (nullptr == m_pInsterface)
			return;

		cout << m_szName << "�� " << endl;
		m_pInsterface->Skill_0();
		m_pInsterface->Skill_1();
		m_pInsterface->Skill_2();
		cout  << endl;
	}

public:
	virtual void Free()
	{
		CMonster::Free();
		cout << "�׽����� ����" << endl;
	}
};

class CAngkara final : public CMonster
{
public:
	CAngkara() {}
	CAngkara(const char* _pszName, CInterface* _pInterface) : CMonster(_pszName, _pInterface) {}
	CAngkara(const CAngkara& _rhs) : CMonster(_rhs) {}
	CAngkara(CAngkara&& _rhs) : CMonster(_rhs) {}
	~CAngkara() = default;

public:
	// �������̽��� ���� ����� �޶���
	void Action()
	{
		if (nullptr == m_pInsterface)
			return;

		cout << m_szName << "�� " << endl;
		m_pInsterface->Skill_0();
		m_pInsterface->Skill_1();
		m_pInsterface->Skill_2();
		cout << "-------------------------------" << endl;
	}

public:
	virtual void Free()
	{
		CMonster::Free();
		cout << "��ī�� ����" << endl;
	}
};

class CManager final : public CBase
{
	static CManager* m_pInstace;

public:
	static CManager* Get_Instance() 
	{
		if (nullptr == m_pInstace)
			m_pInstace = new CManager;

		return m_pInstace;
	}

public:
	// �������̽� Swap
	void Swap_Insterface(CMonster* _pMonster_0, CMonster* _pMonster_1)
	{
		CInterface* pTemp = _pMonster_0->Get_Insterface();
		_pMonster_0->Set_Insterface(_pMonster_1->Get_Insterface());
		_pMonster_1->Set_Insterface(pTemp);
	}

public:
	void Free() 
	{
		cout << "�Ŵ��� ����" << endl;
	}

};

CManager* CManager::m_pInstace = nullptr;

int main()
{
	CMonster* pTestudo = new CTestudo("�׽�����", new CInterface_Testudo);
	CMonster* pAngkara = new CAngkara("��ī��", new CInterface_Angkara);

	// �׼� ����
	pTestudo->Action();
	pAngkara->Action();

	// �������̽� ��ü
	cout << "-- �������̽�  Swap --" << endl << endl;
	CManager::Get_Instance()->Swap_Insterface(pTestudo, pAngkara);

	// �������̽� ��ü �� �׼� ����
	pTestudo->Action();
	pAngkara->Action();

	// ����
	pTestudo->Set_Release();
	pAngkara->Set_Release();
	CManager::Get_Instance()->Set_Release();
	return 0;
}