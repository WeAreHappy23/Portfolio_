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
	void Set_Shared(shared_ptr<CTest>& _spShared) { m_spShared = _spShared; }

public:
	~CTest() 
	{ 
		cout << "�Ҹ��� " << m_iNumber << endl; 
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
	// shared_ptr Ư¡

	// 1. ���� - unique_ptr�� ���������� make_shared<T>�� new�� ������ �� �ִ�
	shared_ptr<CTest> spCreate = make_shared<CTest>(100);
	shared_ptr<CTest> spCreate2(new CTest);	

	// 2. ������ ���� - unique_ptr�� ���������� ���ο� ->, *�� ���� �����ε��� �Ǿ��ִ�.
	spCreate->Show();
	(*spCreate2).Show();

	// 3. ������ ���� - spCreate�� ���� ������ �ϳ� �����Ѵ�. spShared�� ���� �����͸� �����ϹǷ� spCreate�� ���� ������ ����.
	shared_ptr<CTest> spShared = spCreate;		// ���� ������ ��ü�� ������ �ؾ��ϱ� ������ ���� ���簡 �Ͼ��.

	// 4. shared_ptr�� �Ű������� �� ��
	Ref(spShared);		// ���� �������� �Ű������� �ָ� ���� ī��Ʈ�� �þ�� �ʴ´�. �Լ� ������ ������ �����ϴ� �Ͱ� ���� ������ ����
	Value(spShared);	// �� �������� �����ϸ� ���簡 �Ͼ ����ī��Ʈ�� �þ��. 

	// 5. reset()�Լ��� ���� ����
	spShared.reset();		// ���� ī��Ʈ�� 2�� ���¿��� 1���� ��Ų��. ���� 1�λ��¿��ٸ� 0�̵ǰ� ������ �����Ͱ� �޸� �����ȴ�.

	// 6. Ŀ�� ����
	spCreate->Set_Shared(spCreate2);	
	spCreate2->Set_Shared(spCreate);		// spCreate�� spCreate2�� ���� ī��Ʈ�� 2���ȴ�. �������� ������ 1�λ����̱� ������ �޸𸮰� �������� �ʴ´�.

	cout << endl;
	return 0;
}