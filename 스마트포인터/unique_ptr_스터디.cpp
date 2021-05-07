#include <iostream>

using namespace std;

class CTest
{
private:
	int m_iNumber = 0;

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

public:
	~CTest()
	{
		cout << "소멸자 " << m_iNumber << endl;
	}
};

int main()
{
	// unique_ptr의 특징
	// 1. unique_ptr객체는 원시 포인터를 가질 수 있다. 메모리 해제의 책임도 가진다.
	CTest* pClass = new CTest;					// 기본 생성자 호출
	unique_ptr<CTest> upClass(pClass);		// 원시 포인터를 귀속해 메모리 해제의 책임을 가지게된다.

	// 2. unique_ptr은 복사생성자, 대입연산자의 구현을 허용하지 않는다.
	// unique_ptr<CTest> upAnotherRef = upClass;		// unique_ptr은 복사생성자 및  대입연산자에 대한 구현이 없다. (memory 2337번째 줄)

	// 3. 생성 방법
	unique_ptr<CTest> upCreate = make_unique<CTest>();		// 1. C++14에서 생성방식 make_unique<T>.
	unique_ptr<CTest> upCreate2(new CTest);						// 2. 생성자의 매개변수로 전달 (지양할 것. 76라인)

	// 소유권 이전. 이전한 unique_ptr은 nullptr가 된다.
	unique_ptr<CTest> upMove = move(upCreate2);			// 소유권 이전이기 때문에 생성자와 소멸자의 추가가 없음

	// 객체의 멤버 함수 사용. unique_ptr변수명이 포인터인 것은 아니다. 오버로딩이 되어 있기 때문.
	upMove->Show();			// 내부적으로 ->와 *연산자가 오버로딩되어 있기 때문에 사용가능 (memory 2305번째 줄)
	(*upMove).Show();

	// 원시 포인터 삭제 및 받아들이기.
	upCreate.reset(new CTest(100));		// 매개변수가 없으면 가지고 있던 포인터만 해제

	// 포인터 내보내기. 포인터를 해제하지 않고 반환한다.
	CTest* pAccept = upCreate.release();			// upCreate는 원시 포인터를 가지고 있지 않게 된다.
	delete pAccept;

	// unique_ptr의 변수 명을 사용하면 내부의 get()함수에 의해 가지고 있는 포인터를 반환한다. (memory 2310번 째 줄)
	if (nullptr == upCreate)
		cout << "포인터 없음" << endl;

	// 오직 하나만의 unique_ptr이 하나의 포인터를 가져야한다. 안전한 코딩이 될 수 있도록 new보다는 make_unique<T>를 사용해야한다고 생각한다.
	int* pInt = new int;
	unique_ptr<int> upSamePtr1(pInt);		// upSamePtr2가 먼저 해제된 후 다시 또 해제를 하기 때문에 에러가 발생한다.
	// unique_ptr<int> upSamePtr2(pInt);	// 주석 해제 시, 같은 포인터를 두 번 해제하게 된다.

	cout << endl;
	return 0;
}