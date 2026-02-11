// copilot: disable
#include <iostream>
using namespace std;

class BrassAccount
{
private:
	uint64_t balance;
	uint64_t account_number;
	string client_name;

public:
	static uint64_t next_account_number;
	BrassAccount(string name, uint64_t bal);
	bool deposite(uint64_t amount);
	virtual bool withdraw(uint64_t amount);
	virtual void display() const;
	virtual ~BrassAccount() {}
};

BrassAccount::BrassAccount(string name, uint64_t bal) : client_name(std::move(name)), balance(bal)
{
	account_number = next_account_number++;
}

bool BrassAccount::deposite(uint64_t amount)
{
	balance += amount;
	return true;
}

bool BrassAccount::withdraw(uint64_t amount)
{
	if (amount > balance)
		return false;
	balance -= amount;
	return true;
}

void BrassAccount::display() const
{
	cout << "Client name:" << client_name << endl;
	cout << "Account number:" << account_number << endl;
	cout << "Balance:" << balance << endl;
}

class BrassPlusAccount : public BrassAccount
{
private:
	uint64_t overdraft_limit;
	uint16_t rate;

public:
	BrassPlusAccount(string name, uint64_t bal = 500, uint16_t rate = 11125, uint64_t overdraft_limit = 1000) : BrassAccount(name, bal), rate(rate), overdraft_limit(overdraft_limit) {}
	void display() const override;
	bool withdraw(uint64_t amount) override;
};

void BrassPlusAccount::display() const
{
	cout << "overdraft limit:" << overdraft_limit << endl;
	cout << "rate:" << rate << endl;
	BrassAccount::display();
}

bool BrassPlusAccount::withdraw(uint64_t amount)
{
	return BrassAccount::withdraw(amount);
}

uint64_t BrassAccount::next_account_number = 1;

int main(int argc, char const *argv[])
{
	BrassAccount *ba = new BrassAccount("vzgoll", 1000);
	ba->display();
	cout << "-------------------" << endl;
	BrassAccount *bpa = new BrassPlusAccount("ukgoll", 2000);
	bpa->display();
	delete ba;
	delete bpa;
	return 0;
}
