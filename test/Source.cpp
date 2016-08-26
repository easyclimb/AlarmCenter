#include <fstream>
#include <sstream>
#include <set>

int main(int argc, char** argv) 
{
	std::stringstream ss;
	ss << std::ifstream("C:/a.txt").rdbuf();
	std::set<std::string> set;

	{
		std::string line;
		while (std::getline(ss, line)) {
			set.insert(line);
		}
	}
	
	std::ofstream out("C:/a.txt");
	for (auto line : set) {
		out << line << std::endl;
	}
	out.close();
}