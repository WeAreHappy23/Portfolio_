#include <iostream>

using namespace std;

class CTest
{
private:
	int m_iNumber = 0;

public:
	CTest()
	{
		cout << "�⺻ ������" << endl;
	}
	CTest(int _iNum)
	{
		cout << "�Ű����� ������ " << m_iNumber << endl;
		m_iNumber = _iNum;
	}
	CTest(const CTest& _rhs)
	{
		cout << "���������" << endl;
		m_iNumber = _rhs.m_iNumber;
	}
	CTest(CTest&& _rhs)
	{
		cout << "�̵�������" << endl;
		m_iNumber = _rhs.m_iNumber;
	}

public:
	void Show()
	{
		cout << "�Լ� ȣ��" << endl;
	}

public:
	~CTest()
	{
		cout << "�Ҹ��� " << m_iNumber << endl;
	}
};

int main()
{
	// unique_ptr�� Ư¡
	// 1. unique_ptr��ü�� ���� �����͸� ���� �� �ִ�. �޸� ������ å�ӵ� ������.
	CTest* pClass = new CTest;					// �⺻ ������ ȣ��
	unique_ptr<CTest> upClass(pClass);		// ���� �����͸� �ͼ��� �޸� ������ å���� �����Եȴ�.

	// 2. unique_ptr�� ���������, ���Կ������� ������ ������� �ʴ´�.
	// unique_ptr<CTest> upAnotherRef = upClass;		// unique_ptr�� ��������� ��  ���Կ����ڿ� ���� ������ ����. (memory 2337��° ��)

	// 3. ���� ���
	unique_ptr<CTest> upCreate = make_unique<CTest>();		// 1. C++14���� ������� make_unique<T>.
	unique_ptr<CTest> upCreate2(new CTest);						// 2. �������� �Ű������� ���� (������ ��. 76����)

	// ������ ����. ������ unique_ptr�� nullptr�� �ȴ�.
	unique_ptr<CTest> upMove = move(upCreate2);			// ������ �����̱� ������ �����ڿ� �Ҹ����� �߰��� ����

	// ��ü�� ��� �Լ� ���. unique_ptr�������� �������� ���� �ƴϴ�. �����ε��� �Ǿ� �ֱ� ����.
	upMove->Show();			// ���������� ->�� *�����ڰ� �����ε��Ǿ� �ֱ� ������ ��밡�� (memory 2305��° ��)
	(*upMove).Show();

	// ���� ������ ���� �� �޾Ƶ��̱�.
	upCreate.reset(new CTest(100));		// �Ű������� ������ ������ �ִ� �����͸� ����

	// ������ ��������. �����͸� �������� �ʰ� ��ȯ�Ѵ�.
	CTest* pAccept = upCreate.release();			// upCreate�� ���� �����͸� ������ ���� �ʰ� �ȴ�.
	delete pAccept;

	// unique_ptr�� ���� ���� ����ϸ� ������ get()�Լ��� ���� ������ �ִ� �����͸� ��ȯ�Ѵ�. (memory 2310�� ° ��)
	if (nullptr == upCreate)
		cout << "������ ����" << endl;

	// ���� �ϳ����� unique_ptr�� �ϳ��� �����͸� �������Ѵ�. ������ �ڵ��� �� �� �ֵ��� new���ٴ� make_unique<T>�� ����ؾ��Ѵٰ� �����Ѵ�.
	int* pInt = new int;
	unique_ptr<int> upSamePtr1(pInt);		// upSamePtr2�� ���� ������ �� �ٽ� �� ������ �ϱ� ������ ������ �߻��Ѵ�.
	// unique_ptr<int> upSamePtr2(pInt);	// �ּ� ���� ��, ���� �����͸� �� �� �����ϰ� �ȴ�.

	cout << endl;
	return 0;
}