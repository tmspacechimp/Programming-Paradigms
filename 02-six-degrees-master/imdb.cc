using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include "string.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string &directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;

  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !((actorInfo.fd == -1) ||
           (movieInfo.fd == -1));
}

struct actorKey
{
  const char *name;
  const void *file;
};

struct movieKey
{
  const char *name;
  const void *file;
  int year;
};

int cmpMovies(const void *key, const void *offset)
{
  
  movieKey *movie = (movieKey *)key;
  int *cur = (int *)offset;
  char *curMovie = (char *)movie->file + *cur;
  int cmpAns = strcmp(movie->name, curMovie);
  if (cmpAns == 0)
  {

    char singleByteYear;
    int curLength = strlen(curMovie) + 1;
    singleByteYear= *((char *)movie->file + curLength + *cur);
    int curYear = (int)singleByteYear + 1900;
    int delta = movie->year - curYear;

    if (delta > 0) return 1;
    if (delta < 0) return -1; 
    else return 0;
  } return cmpAns;
}

int cmpActors(const void *key, const void *offset)
{
  actorKey *actor = (actorKey *)key;
  int *cur = (int *)offset;
  char *curActor = (char *)actor->file + *cur;
  return strcmp(actor->name, curActor);
}

// you should be implementing these two methods right here...
bool imdb::getCredits(const string &player, vector<film> &films) const
{
  int nActors = *((int *)actorFile);
  actorKey key;
  key.name = player.c_str();
  key.file = actorFile;
  int *res = (int *)bsearch(&key, (int *)actorFile + 1, nActors, sizeof(int), cmpActors);
  if (res == NULL) return false;
  int nameLength = (player.length() + 1);
  int namePadding = nameLength % 2;
  int nMoviesOffset = nameLength + namePadding + *res;
  short nMovies = *(short *)((char *)actorFile + nMoviesOffset);
  int moviesOffset = nMoviesOffset + sizeof(short);
  if (moviesOffset % 4 != 0)
    moviesOffset += 2;
  for (int i = 0; i < nMovies; i++)
  {
    film entry;
    int nameOffset = *(int *)((char *)actorFile + moviesOffset + (i * sizeof(int)));
    char *movieName = (char *)movieFile + nameOffset;
    char singleByteYear = *((char *)movieFile + nameOffset + strlen(movieName) + 1);
    entry.title = movieName;
    entry.year = 1900 + (int)singleByteYear;
    films.push_back(entry);
  }
  return true;
}
bool imdb::getCast(const film &movie, vector<string> &players) const
{
  int nMovies = *((int *)movieFile);
  movieKey key;
  key.name = movie.title.c_str();
  key.file = movieFile;
  key.year = movie.year;
  int *res = (int *)bsearch(&key, (int *)movieFile + 1, nMovies, sizeof(int), cmpMovies);
  if (res == NULL) return false;
  int nameLength = (movie.title.length() + 1);
  nameLength++;
  if (nameLength%2!=0) nameLength++;
  int nActorsOffset = nameLength + *res;
  
  //if ((nameLength + sizeof(char)) % 2 != 0) nActorsOffset++;
  
  short nActors = *(short *)((char *)movieFile + nActorsOffset);
  
  int actorsOffset = nActorsOffset + sizeof(short);
  
  if ((actorsOffset - *res) % 4 != 0) actorsOffset += 2; 
 
  for (int i = 0; i < nActors; i++)
  {
    int nameOffset = *(int *)((char *)movieFile + actorsOffset + (i * sizeof(int)));
    
    char *actorName = (char *)actorFile + nameOffset;
   
   // cout<< actorName << endl;
    
    players.push_back(actorName);
  }
  return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM..
const void *imdb::acquireFileMap(const string &fileName, struct fileInfo &info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo &info)
{
  if (info.fileMap != NULL)
    munmap((char *)info.fileMap, info.fileSize);
  if (info.fd != -1)
    close(info.fd);
}
