 
#ifndef _APPLICATIONS_H_
#define _APPLICATIONS_H_

#include <vector>

typedef struct
{
	char path[256];
	char name[256];
} Application;

class Applications
{
    public:
        Applications();
        ~Applications();
		void Launch(Application app);
		Application Get(int i) { return applications.at(i); }
		int Count() { return applications.size(); }
		char *GetName(int i) { return applications.at(i).name; }

	private:
		std::vector<Application> applications;

		bool GetNameFromXML(char *xml, char *name);
		void Search();
};

#endif
