#include <cmath>
#include <cstring>
#include <cstdio>
#include <map>
#include <string>
using namespace std;

typedef union {
	int n;
	float v;
} val;

const char Marks[][4] = {"，", "。", "？", "！", "“", "”"};
#define COHESION	0.25
#define FREEDEGREE	0.535961
#define CHARWIDTH	3
#define MAXWORDLEN	4

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
			*cs = '\0';
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

int generatecandidates(map<string, int>& characters, map<string, map<char ,map<string, val> > >& candidates, const char *clause)
{
	int i, j, len = strlen(clause);
	char word[CHARWIDTH*MAXWORDLEN + 1];
	memset(word, 0, sizeof(word)); 
	for (i = 0; i <= len - CHARWIDTH; i += CHARWIDTH) {
		if (len < CHARWIDTH)
			return 0;
		for (j = 1; j < MAXWORDLEN; j++) {
			if ((len - i)/CHARWIDTH >= j) {
				//record word frequency and character frequency
				strncpy(word, clause+i, j*CHARWIDTH);
				string cur(word);
				if (j > 1) {
					if (candidates.count(cur))
						candidates[cur]['n']["n"].n++;
					else
						candidates[cur]['n']["n"].n = 1;
					// left neighbor character
					string lc("h");
					char ch[CHARWIDTH];
					if (i > 0) {
						strncpy(ch, clause+i-CHARWIDTH, CHARWIDTH);
						lc = string(ch);
					}
					if (candidates[cur]['l'].count(lc))
						candidates[cur]['l'][lc].n++;
					else
						candidates[cur]['l'][lc].n = 1;
					//right neighbor character
					string rc("t");
					if (len - i - (j+1)*CHARWIDTH >= 0) {
						strncpy(ch, clause+i+j*CHARWIDTH, CHARWIDTH);
						rc = string(ch);
					}
					if (candidates[cur]['r'].count(rc))
						candidates[cur]['r'][rc].n++;
					else
						candidates[cur]['r'][rc].n = 1;
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

int calcohesion(map<string, int>& characters, map<string, map<char, map<string, val> > >& candidates)
{
	map<string, map<char, map<string, val> > >::iterator it;
	for (it = candidates.begin(); it != candidates.end(); ++it) {
		string word = it->first;
		float v1, v2;
		int len = word.length();
		switch (word.length() / CHARWIDTH) {
			case 2:
				candidates[word]['f']["c"].v = candidates[word]['n']["n"].n * candidates[word]['n']["n"].n * 1.0f / (characters[word.substr(0, CHARWIDTH)] * characters[word.substr(CHARWIDTH, CHARWIDTH)]);
				break;
			case 3:
			{
				int count = characters[word.substr(0, CHARWIDTH)];
				v1 = candidates[word]['n']["n"].n * candidates[word]['n']["n"].n * 1.0f / (count * candidates[word.substr(CHARWIDTH, CHARWIDTH*2)]['n']["n"].n);
				count = characters[word.substr(2*CHARWIDTH, CHARWIDTH)];
				v2 = candidates[word]['n']["n"].n * candidates[word]['n']["n"].n * 1.0f / (candidates[word.substr(0, 2*CHARWIDTH)]['n']["n"].n * count);
				candidates[word]['f']["c"].v = v1 < v2 ? v1 : v2;
			}
			case 4:
			v1 = candidates[word]['n']["n"].n * candidates[word]['n']["n"].n * 1.0f / (characters[word.substr(0, CHARWIDTH)] * candidates[word.substr(CHARWIDTH, 3*CHARWIDTH)]['n']["n"].n);
			v2 = candidates[word]['n']["n"].n * candidates[word]['n']["n"].n * 1.0f / (characters[word.substr(0, 3*CHARWIDTH)] * candidates[word.substr(3*CHARWIDTH, CHARWIDTH)]['n']["n"].n);
			candidates[word]['f']["c"].v = candidates[word]['n']["n"].n * 1.0f * candidates[word]['n']["n"].n / (characters[word.substr(0, 2*CHARWIDTH)] * candidates[word.substr(2*CHARWIDTH, 2*CHARWIDTH)]['n']["n"].n);
			if (v1 > v2)
				v1 = v2;
			if (candidates[word]['f']["c"].v > v1)
				candidates[word]['f']["c"].v = v1;
		}
		candidates[word]['f']["c"].v = sqrt(candidates[word]['f']["c"].v);
	}
	return 0;
}

int calfreedegree(map<string, map<char ,map<string, val> > >& candidates)
{
	map<string, map<char, map<string, val> > >::iterator iter;
	for (iter = candidates.begin(); iter != candidates.end(); iter++) {	   
	   map<string, val> lnc = iter->second['l'];
	   map<string, val> rnc = iter->second['r'];
	   map<string, val>::iterator lit, rit;
	   float lce = 0.0f, rce = 0.0f;
	   int n = candidates[iter->first]['n']["n"].n;
	   for (lit = lnc.begin(), rit = rnc.begin(); lit != lnc.end() && rit != rnc.end(); lit++, rit++) {
		   float p = lit->second.n * 1.0f/n;
		   lce = - p * (float)log(p);
		   p = rit->second.n * 1.0f/n;
		   rce = - p * (float)log(p);
	   }
	   candidates[iter->first]['f']["f"].v = lce < rce? lce : rce;
	}
	return 0;
}

int printcandidatesbythreshold(map<string, map<char, map<string, val> > >& candidates, float cohesion, float freedegree)
{
	map<string, map<char, map<string, val> > >::iterator iter;
	for (iter = candidates.begin(); iter != candidates.end(); iter++)
		if (iter->second['f']["c"].v > cohesion && iter->second['f']["f"].v > freedegree)
			printf("%s\t%f\t%f\n", iter->first.c_str(), iter->second['f']["c"].v, iter->second['f']["f"].v);
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp = fopen(argv[1], "r");
	map<string, int> characters;
	map<string, map<char, map<string, val> > > candidates;
	char buf[BUFSIZ];
	memset(buf, 0, sizeof(buf));
	while (fgetsentence(buf, BUFSIZ, fp) != NULL) {
		//printf("%s\n", buf);
		if (strlen(buf) == 0)
			continue;
		generatecandidates(characters, candidates, buf);
		memset(buf, 0, BUFSIZ);
	}
	printf("Start to calculate cohesion...\n");
	calcohesion(characters, candidates);
	printf("Start to calculate freedegree...\n");
	calfreedegree(candidates);
	printcandidatesbythreshold(candidates, COHESION, FREEDEGREE);
	
	return 0;
}
