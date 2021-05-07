#include <iostream>

using namespace std;

class CTest
{
private:
	int m_iNumber = 0;
	shared_ptr<CTest> m_spShared;
	weak_ptr<CTest> m_wpWeak;

public:
	CTest() 
	{ 
		cout << "기본 생성자" << endl; 
	}
	CTest(int _iNum) 
	{ 
		cout << "매개변수 생성자 " << m_iNumber << endl; 
		m_iNumber = _iNum;  
	}
	CTest(const CTest& _rhs) 
	{ 
		cout << "복사생성자" << endl; 
		m_iNumber = _rhs.m_iNumber;
	}
	CTest(CTest&& _rhs) 
	{ 
		cout << "이동생성자" << endl;  
		m_iNumber = _rhs.m_iNumber;
	}

public:
	void Show() 
	{
		cout << "함수 호출" << endl;
	}
	void Set_Weak(shared_ptr<CTest>& _spShared) { m_wpWeak = _spShared; }

public:
	~CTest() 
	{ 
		cout << "소멸자 " << m_iNumber << endl; 
	}
};

int main()
{
	// weak_ptr 특징

	// 1. shared_ptr의 커플 문제를 방지하기 위해서 사용한다. 강한 참조가 아닌 약참조 횟수를 증가시키며 shared_ptr 해제에는 영향을 주지 않는다.
	shared_ptr<CTest> spCreate = make_shared<CTest>(100);
	shared_ptr<CTest> spCreate2 = make_shared<CTest>(23);
	spCreate->Set_Weak(spCreate2);
	spCreate2->Set_Weak(spCreate);

	// 2. weak_ptr은 단독으로 생성하지 못한다. shared_ptr을 참조해서 사용할 수 있다.
	shared_ptr<CTest> spInt = make_shared<CTest>(32);
	weak_ptr<CTest> wpInt = spInt;
	
	// 3. weak_ptr은 단독으로 사용하지는 못하지만 lock함수로 원시 포인터 기능을 사용할 수 있다. 이 때, spCreate3의 강한 참조는 1증가한다.
	shared_ptr<CTest> spCreate3 = wpInt.lock();		// 만약 wpInt이 유효하지 않다면 empty상태인 포인터를 반환한다.

	// 4. 위의 기능은 멀티스레드에서 안정성을 준다. 강한 참조 횟수를 1증가시켜 다른 스레드로부터의 삭제를 방지할 수 있기 때문
	if (nullptr != spCreate3) {}		// 삭제되지 않았을 때
	else {}		// 삭제됐을 때

	// 5. weak_ptr의 유효한지를 판단. 
	if (false == wpInt.expired()) {}		// expired상태가 아니면 작업을 수행한다.

	cout << endl;
	return 0;
}