#include <cstring>
#include <map>
#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

const char Marks[][4] = {"，", "。", "？", "！"};
#define CHARWIDTH 3
#define MAXWORDLEN 4

int ismark(char *s, size_t n)
{
	int i;
	for (i = 0; i < sizeof(Marks)/sizeof(Marks[0]); i++)
		if (!strncmp(Marks[i], s, n)) 
			return 1;
	
	return 0;
}

char *fgetsentence(char *s, int n, FILE *iop)
{
	register int c;
	register char ch[CHARWIDTH + 1];

	register char *cs = s;
	while (--n > 0 && ( c = getc(iop)) != EOF) {
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
			continue;
		int i = 0;
		while (c & 0200 >> i) 
			++i;
		if (i == 0) { // single byte ascii character
			*cs++ = c;
			continue;
		}
		int width = i;
		if (width > 2)
			ch[0] = c;
		for (i = 1; i < width; i++)
			ch[i] = c = getc(iop);
		if (ismark(ch, width))
			break;
		else
			for (i = 0; i < width; i++)
				*cs++ = ch[i];
	}
	return (c == EOF && cs == s) ? NULL : s;
}

int generatecandidates(map<string, int>& characters, map<string, map<string, int> >& candidates, const char *clause)
{
	int i, j, len = strlen(clause);
	char word[CHARWIDTH*MAXWORDLEN];
	memset(word, 0, CHARWIDTH*MAXWORDLEN); 
	for (i = 0; i <= len - CHARWIDTH; i += CHARWIDTH) {
		if (len < CHARWIDTH)
			return 0;
		strncpy(word, clause+CHARWIDTH*i, CHARWIDTH);
		for (j = 1; j < MAXWORDLEN; j++) {
			if ((len - i)/CHARWIDTH >= j) {
				//record word frequency and character frequency
				strncpy(word, clause+CHARWIDTH*i, j*CHARWIDTH);
				string cur(word);
				if (j > 1) {
					if (candidates.count(cur))
						candidates[cur]["n"]++;
					else
						candidates[cur]["n"] = 1;
					// left neighbor character
					string lc("h");
					char ch[CHARWIDTH];
					if (i > 0) {
						strncpy(ch, clause+CHARWIDTH*(i-1), CHARWIDTH);
						lc = string(ch);
					}
					if (candidates[cur].count(lc))
						candidates[cur][lc]--;
					else
						candidates[cur][lc] = -1;
					//right neighbor character
					string rc("t");
					if (len - (i+j+1)*CHARWIDTH >= 0) {
						strncpy(ch, clause+(i+j)*CHARWIDTH, CHARWIDTH);
						rc = string(ch);
					}
					if (candidates[cur].count(rc))
						candidates[cur][rc]++;
					else
						candidates[cur][rc] = 1;
				} else {
					if (characters.count(cur))
						characters[cur]++;
					else
						characters[cur] = 1;
				}
			}
		}
	}
	return i;
}

int calcohesion(map<string, int>& characters, map<string, map<string, int> >& candidates)
{
	map<string, int> >::iterator it;
	for (it = candidates.begin(); it != candidates.end(); ++it) {
		string word = it->firtst;
		float v1, v2;
		int len = word.length();
		switch (word.length() / CHARWIDTH) {
			case 2:
				candidates[word]["c"] = candidates[word]["n"] *1000 / (characters[word.substr(0, CHARWIDTH)] * characters[word.substr(CHARWIDTH, CHARWIDTH)]);
				break;
			case 3:
			{
				int count = characters[word.substr(0, CHARWIDTH)];
				v1 = candidates[word]["n"] * candidates[word]["n"] * 1.0f / (count * candidates[word.substr(CHARWIDTH, CHARWIDTH*2)]);
				count = characters[word.substr(2*CHARWIDTH, CHARWIDTH)];
				v2 = candidates[word]["n"] * candidates[word]["n"] * 1.0f / (candidates[word.substr(0, 2*CHARWIDTH)] * count);
				candidates[word]["c"] = v1 < v2 ? (int)(1000*v1) : (int)(1000*v2);
			}
			case 4:
			v1 = candidates[word]["n"] * candidates[word]["n"] * 1.0f / (characters[word.substr(0, CHARWIDTH)] * candidats[word.substr(CHARWIDTH, 3*CHARWIDTH)]);
			v2 = candidates[word]["n"] * candidates[word]["n"] * 1.0f / (characters[word.substr(0, 3*CHARWIDTH)] * candidats[word.substr(3*CHARWIDTH, CHARWIDTH)]);
			candidates[word]["c"] = candidates[word]["n"] * candidates[word]["n"] * 1000 / (characters[word.substr(0, 2*CHARWIDTH)] * candidats[word.substr(2*CHARWIDTH, 2*CHARWIDTH)]);
			if (v1 > v2)
				v1 = v2;
			if (candidates[word]["c"] > v1)
				candidates[word]["c"] = (int)(1000*v1);
		}
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp = fopen(argv[1], "r");
	map<string, int> characters;
	map<string, map<string, int> > candidates;
	char buf[BUFSIZ];
	memset(buf, 0, sizeof(buf));
	while (fgetsentence(buf, BUFSIZ, fp) != NULL) {
		printf("%s\n", buf);
		generatecandidates(characters, candidates, buf);
		memset(buf, 0, BUFSIZ);
	}
	
	return 0;
}
