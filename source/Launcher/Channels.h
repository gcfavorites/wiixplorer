
#ifndef _CHANNELS_H_
#define _CHANNELS_H_

#include <vector>

typedef struct
{
	u64  title;
	char name[256];
} Channel;

class Channels
{
    public:
        Channels();
        ~Channels();

		void Launch(Channel channel);
		Channel Get(int i) { return channels.at(i); }
		int Count() { return channels.size(); }
		char *GetName(int i) { return channels.at(i).name; }

	private:
		std::vector<Channel> channels;

		bool GetNameFromApp(u64 title, char *name);
		u64* GetChannels(u32* cnt);
		void Search();
};

#endif
