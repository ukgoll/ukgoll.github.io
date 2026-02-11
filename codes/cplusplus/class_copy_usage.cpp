#include <iostream>
using namespace std;

class CopyClass{
private:
	int* numPointer;
	std::string name;
public:
	CopyClass(){
		numPointer = new int(42);
		name = "DefaultName";
	}

	CopyClass(const CopyClass& other){
		std::cout << "Copy constructor called" << std::endl;
		numPointer = new int(*(other.numPointer) + 1);
	}

	int getValue() const {
		return *numPointer;
	}
	const CopyClass& operator=(const CopyClass& other){
		std::cout << "Invoke copy assignment operator" << std::endl;
		if(this != &other){
			if(numPointer != nullptr) delete numPointer;
			numPointer = new int(*(other.numPointer));
		}
		return *this;
	}

	~CopyClass(){
		std::cout << "Destructor called for value: " << *numPointer << std::endl;
		delete numPointer;
	}
};

void test_copy(CopyClass obj){
// void test_copy(const CopyClass &obj){
	std::cout << "In test copy CopyClass value: " << obj.getValue() << std::endl;
}

int main(){
	CopyClass original = CopyClass();
	std::cout << "Original CopyClass value: " << original.getValue() << std::endl;
	test_copy(original);
	std::cout << "Original CopyClass value: " << original.getValue() << std::endl;
	return 0;
}