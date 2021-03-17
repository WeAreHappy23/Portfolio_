// ������ ����

// ������ ���ϰ��� �޸� �������� ���·� �̺�Ʈ�� �����ϴ� ����
// ��ϵ� ��ҵ��� ��ȸ�ϸ鼭 �̺�Ʈ�� �����Ѵ�

// ��ϵ� ��ü�� ���ؼ� �̺�Ʈ�� ������ �� �ִ�.

// ����
// 1. ���͸� �Ŵ����� ���
// 2. �Ŵ����� �̺�Ʈ�� �����ϸ� ���ʹ� ������ �������� �ο�����

// ���� ��
// 1. ���ݱ����� ������Ʈ������ ������ ������ ����ؼ� �������� ����� �̺�Ʈ�� ����
// 2. ������ Ŭ������ ��ü�� ���� ���ǿ��� �̺�Ʈ�� ������� ������ �����ϱ� ���ؼ��� ������������ ����ϸ� ȿ�����̶�� ����
// 3. �������δ� ��ϵ� ��ü�� ���ؼ� ������ ������ �ʿ��ϴٰ� ���� (��ü ��������� ������ ������ ������ �ʴ� ����� ���� ���ɼ��� �ִٰ� ����)

#include<iostream>

using namespace std;

#include <list>
#include <time.h>

enum BUFF { ATTACK, SHIELD, SPPED, IMMUNITY, INVINCIBLE, RECOVERY, BUFF_END };

class CBase abstract
{
private:
	int				m_iRefCnt = 0;

protected:
	explicit CBase() {}
	virtual ~CBase() = default;

public:
	void Set_AddRef() { ++m_iRefCnt; }
	void Set_ReleaseRef()
	{
		// �����ϴ� ��ü�� ���� ��� ����
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

class CObserver : public CBase
{
protected:
	char*			m_szName = nullptr;					// ���� �̸�
	BUFF			m_eBuff = BUFF::BUFF_END;			// ���Ϳ��� �ο��� ����

public:
	CObserver() {};
	CObserver(const char* _szName)
	{
		m_szName = new char[strlen(_szName) + 1];
		strcpy(m_szName, _szName);
	};
	CObserver(const CObserver& _rhs) 
	{
		if(nullptr != m_szName)
			delete[] m_szName;

		m_szName = new char[strlen(_rhs.m_szName) + 1];
		strcpy(m_szName, _rhs.m_szName);

		m_eBuff = _rhs.m_eBuff;
	};
	CObserver(CObserver&& _rhs)
	{
		m_szName = _rhs.m_szName;
		m_eBuff = _rhs.m_eBuff;

		_rhs.m_szName = nullptr;
	};
	~CObserver() = default;

public:
	void Set_Buff(CObserver* _pObserver, BUFF _eBuff)
	{
		switch (_eBuff)
		{
		case ATTACK:
			cout << m_szName << ", ���ݷ� ��ȭ ������ ������ϴ�.";
			break;
		case SHIELD:
			cout << m_szName << ", ��ȣ�� ������ ������ϴ�.";
			break;
		case SPPED:
			cout << m_szName << ", �ӵ� ���� ������ ������ϴ�.";
			break;
		case IMMUNITY:
			cout << m_szName << ", �鿪 ������ ������ϴ�.";
			break;
		case INVINCIBLE:
			cout << m_szName << ", 1�� �� ���� ������ ������ϴ�.";
			break;
		case RECOVERY:
			cout << m_szName << ", ȸ�� ������ ������ϴ�.";
			break;
		}

		cout << "(" << _pObserver->Get_Name() << " ����)" << endl;
	}
	const char* Get_Name() { return m_szName; }

public:
	void Free()
	{
		delete[] m_szName;
	}

}COBSERVER;

class CSubject abstract : public CBase
{
protected:
	CSubject() {};
	~CSubject() {};

protected:
	virtual void Register_Main(CObserver* _pMain_Monster) = 0;
	virtual void Register_Observer(CObserver* _pObserver) = 0;
	virtual void Release_Observer(CObserver* _pObserver) = 0;
	virtual void Notify_Buff() = 0;
};

class CManager final : public CSubject
{
private:
	list<CObserver*>		m_listObserver;
	CObserver*				m_pMain_Monster = nullptr;

public:
	CManager() {}
	~CManager() = default;

public:
	// ������ �� ���� ���
	virtual void Register_Main(CObserver* _pMain_Monster) override
	{
		if(nullptr != m_pMain_Monster)
			m_pMain_Monster->Set_ReleaseRef();

		m_pMain_Monster = _pMain_Monster;
		m_pMain_Monster->Set_AddRef();
	}
	// ������ ���� ���� ���
	virtual void Register_Observer(CObserver* _pObserver) override
	{
		if (true == Check_Pointer(_pObserver))
			return;

		m_listObserver.emplace_back(_pObserver);
		_pObserver->Set_AddRef();
	}
	// ��ϵ� ���� ����
	virtual void Release_Observer(CObserver* _pObserver) override
	{
		list<CObserver*>::iterator iter = m_listObserver.begin();

		for (; iter != m_listObserver.end() ; ++iter)
		{
			if (_pObserver == (*iter))
			{
				(*iter)->Set_ReleaseRef();
				m_listObserver.erase(iter);
				return;
			}
		}
	}
	// �̺�Ʈ �ο�
	virtual void Notify_Buff() override
	{
		// ������ �� ���Ͱ� ������ �Լ��� ����������
		if (nullptr == m_pMain_Monster)
			return;

		BUFF eBuff = BUFF::BUFF_END;
		for (auto& pObserver : m_listObserver)
		{
			pObserver->Set_Buff(m_pMain_Monster, BUFF(rand() % int(BUFF::BUFF_END)));
		}
	}
	// �ߺ� ��ϵǾ����� Ȯ��
	bool Check_Pointer(CObserver* _pObserver)
	{
		for (auto& pObserver : m_listObserver)
		{
			if (_pObserver == pObserver)
				return true;
		}
		return false;
	}

public:
	void Free()
	{
		for (auto& pObserver : m_listObserver)
			pObserver->Set_ReleaseRef();

		m_pMain_Monster->Set_ReleaseRef();
	}
}CMANAGER;

int main()
{
	srand(unsigned(time(NULL)));

	CObserver* pMain_Monster_Caides = new CObserver("ī�̵���");
	CObserver* pMain_Monster_Rubellite = new CObserver("�级����Ʈ");

	CObserver* pObserver[6] = {
		new CObserver("�׽�����"),
		new CObserver("����"),
		new CObserver("�÷���"),
		new CObserver("����"),
		new CObserver("������"),
		new CObserver("��ī��"),
	};

	CManager* pManager = new CManager;
	pManager->Register_Main(pMain_Monster_Caides);
	pManager->Register_Observer(pObserver[0]);
	pManager->Register_Observer(pObserver[1]);
	pManager->Register_Observer(pObserver[2]);
	pManager->Register_Observer(pObserver[3]);
	pManager->Register_Observer(pObserver[4]);
	pManager->Register_Observer(pObserver[5]);

	pManager->Notify_Buff();		// �̺�Ʈ �߻�

	cout << endl;
	cout << "�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�" << endl;

	pManager->Release_Observer(pObserver[2]);
	pManager->Release_Observer(pObserver[3]);
	pManager->Release_Observer(pObserver[5]);
	pManager->Register_Main(pMain_Monster_Rubellite);
	pManager->Notify_Buff();		// �̺�Ʈ �߻�

	cout << endl;

	// ����
	pManager->Set_ReleaseRef();
	for (auto& pobserver_element : pObserver)
		pobserver_element->Set_ReleaseRef();
	pMain_Monster_Caides->Set_ReleaseRef();
	pMain_Monster_Rubellite->Set_ReleaseRef();

	
	return 0;
}