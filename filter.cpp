#include <pybind11/pybind11.h>
namespace py = pybind11;

struct Pet {
  Pet(const std::string &name) : name(name) {}
  void setName(const std::string &name_) { name = name_; }
  const std::string &getName() const { return name; }

  std::string name;
};

struct Dog : Pet {
  Dog(const std::string &name) : Pet(name) {}
  std::string bark() const { return "woof!"; }
};

int add(int x, int y){
  return x + y;
}

//可以使用class_::def_property()(只读成员使用class_::def_property_readonly())来定义并私有成员，并生成相应的setter和geter方法：
PYBIND11_MODULE(example, m) {
  py::class_<Pet>(m, "Pet")
      .def(py::init<const std::string &>())
      .def_readwrite("name", &Pet::name)
      .def("setName", &Pet::setName)
      .def("getName", &Pet::getName);
  py::class_<Dog, Pet /* <- specify C++ parent type */>(m, "Dog")
      .def(py::init<const std::string &>())
      .def("bark", &Dog::bark);
  m.def("add",&add);
}