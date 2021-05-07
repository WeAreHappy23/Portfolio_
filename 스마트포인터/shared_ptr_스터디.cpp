#include <iostream>

using namespace std;

class CTest
{
private:
	int m_iNumber = 0;
	shared_ptr<CTest> m_spShared;

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
	void Set_Shared(shared_ptr<CTest>& _spShared) { m_spShared = _spShared; }

public:
	~CTest() 
	{ 
		cout << "소멸자 " << m_iNumber << endl; 
	}
};

void Ref(shared_ptr<CTest>& _spRef)
{
}

void Value(shared_ptr<CTest> _spRef)
{
}

int main()
{
	// shared_ptr 특징

	// 1. 생성 - unique_ptr과 마찬가지로 make_shared<T>와 new로 생성할 수 있다
	shared_ptr<CTest> spCreate = make_shared<CTest>(100);
	shared_ptr<CTest> spCreate2(new CTest);	

	// 2. 포인터 접근 - unique_ptr과 마찬가지로 내부에 ->, *에 대한 오버로딩이 되어있다.
	spCreate->Show();
	(*spCreate2).Show();

	// 3. 포인터 공유 - spCreate의 강한 참조가 하나 증가한다. spShared도 같은 포인터를 공유하므로 spCreate의 참조 개수와 같다.
	shared_ptr<CTest> spShared = spCreate;		// 원시 포인터 자체의 공유를 해야하기 때문에 얕은 복사가 일어난다.

	// 4. shared_ptr을 매개변수로 줄 때
	Ref(spShared);		// 참조 형식으로 매개변수를 주면 참조 카운트가 늘어나지 않는다. 함수 내에서 원본에 접근하는 것과 같기 때문에 주의
	Value(spShared);	// 값 형식으로 전달하면 복사가 일어나 참조카운트가 늘어난다. 

	// 5. reset()함수로 참조 해제
	spShared.reset();		// 참조 카운트가 2인 상태에서 1감소 시킨다. 만약 1인상태였다면 0이되고 내부의 포인터가 메모리 해제된다.

	// 6. 커플 문제
	spCreate->Set_Shared(spCreate2);	
	spCreate2->Set_Shared(spCreate);		// spCreate와 spCreate2의 참조 카운트가 2가된다. 스코프가 끝나도 1인상태이기 때문에 메모리가 해제되지 않는다.

	cout << endl;
	return 0;
}