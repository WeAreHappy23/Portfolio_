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
	void Set_Weak(shared_ptr<CTest>& _spShared) { m_wpWeak = _spShared; }

public:
	~CTest() 
	{ 
		cout << "�Ҹ��� " << m_iNumber << endl; 
	}
};

int main()
{
	// weak_ptr Ư¡

	// 1. shared_ptr�� Ŀ�� ������ �����ϱ� ���ؼ� ����Ѵ�. ���� ������ �ƴ� ������ Ƚ���� ������Ű�� shared_ptr �������� ������ ���� �ʴ´�.
	shared_ptr<CTest> spCreate = make_shared<CTest>(100);
	shared_ptr<CTest> spCreate2 = make_shared<CTest>(23);
	spCreate->Set_Weak(spCreate2);
	spCreate2->Set_Weak(spCreate);

	// 2. weak_ptr�� �ܵ����� �������� ���Ѵ�. shared_ptr�� �����ؼ� ����� �� �ִ�.
	shared_ptr<CTest> spInt = make_shared<CTest>(32);
	weak_ptr<CTest> wpInt = spInt;
	
	// 3. weak_ptr�� �ܵ����� ��������� �������� lock�Լ��� ���� ������ ����� ����� �� �ִ�. �� ��, spCreate3�� ���� ������ 1�����Ѵ�.
	shared_ptr<CTest> spCreate3 = wpInt.lock();		// ���� wpInt�� ��ȿ���� �ʴٸ� empty������ �����͸� ��ȯ�Ѵ�.

	// 4. ���� ����� ��Ƽ�����忡�� �������� �ش�. ���� ���� Ƚ���� 1�������� �ٸ� ������κ����� ������ ������ �� �ֱ� ����
	if (nullptr != spCreate3) {}		// �������� �ʾ��� ��
	else {}		// �������� ��

	// 5. weak_ptr�� ��ȿ������ �Ǵ�. 
	if (false == wpInt.expired()) {}		// expired���°� �ƴϸ� �۾��� �����Ѵ�.

	cout << endl;
	return 0;
}