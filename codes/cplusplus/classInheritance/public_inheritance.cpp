#include <iostream>

class Base
{
private:
	int age;
	void test_get_name()
	{
		std::cout << "Base class test_get_name called." << std::endl;
	}

	// void setAge(int a){
	// 	age = a;
	// }
public:
	Base(int age) : age(age)
	{
		std::cout << "Base class constructor invoked." << std::endl;
	}
	virtual ~Base()
	{
		std::cout << "Base class destructor invoked." << std::endl;
	}
	void set_age(int a)
	{
		age = a;
	}
	int get_age() const
	{
		return age;
	}
};

class Derived : public Base
{
private:
	std::string name;

public:
	Derived(int age, std::string name) : Base(age), name(std::move(name))
	{
		std::cout << "Derived class constructor invoked." << std::endl;
	}
	~Derived()
	{
		std::cout << "Derived class destructor invoked." << std::endl;
	}
	using Base::get_age;
	using Base::set_age;
	void derived_test_get_name()
	{
		std::cout << "Derived name is: " << name << std::endl;
		// test_get_name();
	}
};

int main(int argc, char const *argv[])
{
	using namespace std;
	Base *d = new Derived(10, "vzgoll");
	std::cout << "d age is: " << d->get_age() << std::endl;
	d->set_age(30);
	std::cout << "d age is: " << d->get_age() << std::endl;
	// d->derived_test_get_name();
	delete d;
	std::cout << "---------------------------" << std::endl;
	Derived d2 = Derived(20, "ukgoll");
	std::cout << "d2 age is: " << d2.get_age() << std::endl;
	return 0;
}
