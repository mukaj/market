#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include <string>

class element {
public:
	element() {};
	~element() {};
	element(const std::string & name, const unsigned int & cost);
	unsigned int cost() const;
	const std::string & name() const;
	void set_name(const std::string & name);
	void set_cost(const unsigned int & cost);
private:
	std::string name_;
	unsigned int cost_;
};

element::element(const std::string & name, const unsigned int & cost) : name_(name), cost_(cost) {}

inline const std::string & element::name() const {
	return name_;
}

inline unsigned int element::cost() const {
	return cost_;
}

inline void element::set_name(const std::string & name) {
	this->name_ = name;
}

inline void element::set_cost(const unsigned int & cost) {
	this->cost_ = cost;
}
#endif // _ELEMENT_H_