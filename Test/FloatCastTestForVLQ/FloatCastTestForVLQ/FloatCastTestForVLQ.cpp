#include <iostream>

int main()
{
	float tmp = 3.141592;

	uint32_t tmp2 = tmp;

	float tmp3 = tmp2;

	uint32_t tmp4 = 0;

	memcpy_s(&tmp4, sizeof(tmp4), &tmp, sizeof(tmp));

	float tmp5 = 0.0f;

	memcpy_s(&tmp5, sizeof(tmp5), &tmp4, sizeof(tmp4));

	std::cout << "tmp : " << tmp << std::endl;
	std::cout << "tmp2 : " << tmp2 << std::endl;
	std::cout << "tmp3 : " << tmp3 << std::endl;
	std::cout << "tmp4 : " << tmp4 << std::endl;
	std::cout << "tmp5 : " << tmp5 << std::endl;
}