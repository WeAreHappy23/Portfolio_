// 브릿지 패턴

// 상속을 이용한 패턴
// 실제 구현은 인터페이스를 상속받는 클래스에서 한다.

// 기초클래스의 형태로 실객체를 생성하기 때문에 포인터 교체가 가능
// 강한 결합이 아니기 때문에 설계가 유연해진다.

// 구현
// 1. 두 몬스터를 생성할 때 각각 서로 다른 인터페이스를 부여하고 행동 실행
// 2. 인터페이스를 교체하고 행동 실행

// 느낀 점
// 1. 유니티 엔진에서 컴포넌트에 리소스들을 서로 바꿔가며 실행할 수 있는 것과 비슷하다고 느낌
// 2. 처음에 차라리 오버라이딩으로 재정의하면(다형성) 되지 않을까라고 고민했지만, 
//    하나의 클래스에 인터페이스만 바꿔가며 실행하는 것도 좋은 방법이라고 생각
//    (ex. Sword_1, Sword_2처럼 오버라이딩해가면서 클래스를 늘리는 방법보다, Sword라는 클래스에 인터페이스만 바꿔가며 사용하는 것이 효율적이라 느낌)

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
	// 실제 구현은 상속받는 클래스에서 한다
	virtual void Skill_0() = 0;
	virtual void Skill_1() = 0;
	virtual void Skill_2() = 0;

public:
	virtual void Free() {}
	
};

class CInterface_Testudo final : public CInterface
{
public:
	// 실제 구현
	virtual void Skill_0() { cout << "물가에 간다" << endl; }
	virtual void Skill_1() { cout << "물을 마신다" << endl; }
	virtual void Skill_2() { cout << "세수를 한다" << endl; }

public:
	virtual void Free()
	{
		CInterface::Free();

		cout << "테스투도 전용 인터페이스 해제" << endl;
	};
};

class CInterface_Angkara final : public CInterface
{
public:
	// 실제 구현
	virtual void Skill_0() { cout << "먹이를 찾는다" << endl; }
	virtual void Skill_1() { cout << "열매를 발견한다" << endl; }
	virtual void Skill_2() { cout << "열매를 향해 달려간다" << endl; }

public:
	virtual void Free()
	{
		CInterface::Free();

		cout << "앙카라 전용 인터페이스 해제" << endl;
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
	// 추상화된 기능을 호출 -> 인터페이스의 실 객체의 기능을 호출할 수 있음
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
	// 인터페이스를 몬스터와 같이 생성하는 작업을 하기 때문에
	// RefCnt를 감소시키는 작업이 필요
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
	// 인터페이스에 따라 기능이 달라짐
	void Action()
	{
		if (nullptr == m_pInsterface)
			return;

		cout << m_szName << "가 " << endl;
		m_pInsterface->Skill_0();
		m_pInsterface->Skill_1();
		m_pInsterface->Skill_2();
		cout  << endl;
	}

public:
	virtual void Free()
	{
		CMonster::Free();
		cout << "테스투도 해제" << endl;
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
	// 인터페이스에 따라 기능이 달라짐
	void Action()
	{
		if (nullptr == m_pInsterface)
			return;

		cout << m_szName << "가 " << endl;
		m_pInsterface->Skill_0();
		m_pInsterface->Skill_1();
		m_pInsterface->Skill_2();
		cout << "-------------------------------" << endl;
	}

public:
	virtual void Free()
	{
		CMonster::Free();
		cout << "앙카라 해제" << endl;
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
	// 인터페이스 Swap
	void Swap_Insterface(CMonster* _pMonster_0, CMonster* _pMonster_1)
	{
		CInterface* pTemp = _pMonster_0->Get_Insterface();
		_pMonster_0->Set_Insterface(_pMonster_1->Get_Insterface());
		_pMonster_1->Set_Insterface(pTemp);
	}

public:
	void Free() 
	{
		cout << "매니저 종료" << endl;
	}

};

CManager* CManager::m_pInstace = nullptr;

int main()
{
	CMonster* pTestudo = new CTestudo("테스투도", new CInterface_Testudo);
	CMonster* pAngkara = new CAngkara("앙카라", new CInterface_Angkara);

	// 액션 실행
	pTestudo->Action();
	pAngkara->Action();

	// 인터페이스 교체
	cout << "-- 인터페이스  Swap --" << endl << endl;
	CManager::Get_Instance()->Swap_Insterface(pTestudo, pAngkara);

	// 인터페이스 교체 후 액션 실행
	pTestudo->Action();
	pAngkara->Action();

	// 해제
	pTestudo->Set_Release();
	pAngkara->Set_Release();
	CManager::Get_Instance()->Set_Release();
	return 0;
}