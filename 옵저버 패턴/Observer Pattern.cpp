// 옵저버 패턴

// 중재자 패턴과는 달리 수직적인 형태로 이벤트를 전달하는 패턴
// 등록된 요소들을 순회하면서 이벤트를 전달한다

// 등록된 객체에 한해서 이벤트를 전달할 수 있다.

// 구현
// 1. 몬스터를 매니저에 등록
// 2. 매니저가 이벤트를 전달하면 몬스터는 버프를 랜덤으로 부여받음

// 느낀 점
// 1. 지금까지의 프로젝트에서는 중재자 패턴을 사용해서 수평적인 관계로 이벤트를 전달
// 2. 동일한 클래스의 객체라도 일정 조건에서 이벤트를 줘야할지 말지를 결정하기 위해서는 옵저버패턴을 사용하면 효율적이라고 생각
// 3. 단점으로는 등록된 객체에 대해서 엄격한 관리가 필요하다고 생각 (객체 등록해제를 잊으면 다음에 원하지 않는 결과가 생길 가능성이 있다고 생각)

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
		// 참조하는 객체가 없을 경우 삭제
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
	char*			m_szName = nullptr;					// 몬스터 이름
	BUFF			m_eBuff = BUFF::BUFF_END;			// 몬스터에게 부여된 버프

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
			cout << m_szName << ", 공격력 강화 버프를 얻었습니다.";
			break;
		case SHIELD:
			cout << m_szName << ", 보호막 버프를 얻었습니다.";
			break;
		case SPPED:
			cout << m_szName << ", 속도 증가 버프를 얻었습니다.";
			break;
		case IMMUNITY:
			cout << m_szName << ", 면역 버프를 얻었습니다.";
			break;
		case INVINCIBLE:
			cout << m_szName << ", 1턴 간 무적 버프를 얻었습니다.";
			break;
		case RECOVERY:
			cout << m_szName << ", 회복 버프를 얻었습니다.";
			break;
		}

		cout << "(" << _pObserver->Get_Name() << " 버프)" << endl;
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
	// 버프를 줄 몬스터 등록
	virtual void Register_Main(CObserver* _pMain_Monster) override
	{
		if(nullptr != m_pMain_Monster)
			m_pMain_Monster->Set_ReleaseRef();

		m_pMain_Monster = _pMain_Monster;
		m_pMain_Monster->Set_AddRef();
	}
	// 버프를 받을 몬스터 등록
	virtual void Register_Observer(CObserver* _pObserver) override
	{
		if (true == Check_Pointer(_pObserver))
			return;

		m_listObserver.emplace_back(_pObserver);
		_pObserver->Set_AddRef();
	}
	// 등록된 몬스터 해제
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
	// 이벤트 부여
	virtual void Notify_Buff() override
	{
		// 버프를 줄 몬스터가 없으면 함수를 빠져나간다
		if (nullptr == m_pMain_Monster)
			return;

		BUFF eBuff = BUFF::BUFF_END;
		for (auto& pObserver : m_listObserver)
		{
			pObserver->Set_Buff(m_pMain_Monster, BUFF(rand() % int(BUFF::BUFF_END)));
		}
	}
	// 중복 등록되었는지 확인
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

	CObserver* pMain_Monster_Caides = new CObserver("카이데스");
	CObserver* pMain_Monster_Rubellite = new CObserver("루벨라이트");

	CObserver* pObserver[6] = {
		new CObserver("테스투도"),
		new CObserver("드라고나"),
		new CObserver("플루즈"),
		new CObserver("총포"),
		new CObserver("슬라임"),
		new CObserver("앙카라"),
	};

	CManager* pManager = new CManager;
	pManager->Register_Main(pMain_Monster_Caides);
	pManager->Register_Observer(pObserver[0]);
	pManager->Register_Observer(pObserver[1]);
	pManager->Register_Observer(pObserver[2]);
	pManager->Register_Observer(pObserver[3]);
	pManager->Register_Observer(pObserver[4]);
	pManager->Register_Observer(pObserver[5]);

	pManager->Notify_Buff();		// 이벤트 발생

	cout << endl;
	cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << endl;

	pManager->Release_Observer(pObserver[2]);
	pManager->Release_Observer(pObserver[3]);
	pManager->Release_Observer(pObserver[5]);
	pManager->Register_Main(pMain_Monster_Rubellite);
	pManager->Notify_Buff();		// 이벤트 발생

	cout << endl;

	// 해제
	pManager->Set_ReleaseRef();
	for (auto& pobserver_element : pObserver)
		pobserver_element->Set_ReleaseRef();
	pMain_Monster_Caides->Set_ReleaseRef();
	pMain_Monster_Rubellite->Set_ReleaseRef();

	
	return 0;
}