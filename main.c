#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

#include <uk/plat/bootstrap.h>

#define DB_NAME     "database.db"
#define DB_QUERY     "INSERT INTO tab VALUES (null, 'value')"
#define OP_NUM        60000
#define VERIFY_Q    0

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
#ifdef VERIFY_Q
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
#endif
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  rc = sqlite3_open(DB_NAME, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return(1);
  }

  struct timespec start;
  if (clock_gettime(CLOCK_MONOTONIC, &start)) {
    perror("Could not read start time!");
    sqlite3_close(db);
    return 1;
  }
  
  for (int i = 0; i < OP_NUM; i++) {
        rc = sqlite3_exec(db, DB_QUERY, callback, 0, &zErrMsg);

      if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
	break;
      }
  }

  struct timespec end;
  if (clock_gettime(CLOCK_MONOTONIC, &end)) {
    perror("Could not read end time!");
    sqlite3_close(db);
    return 1;
  }

  time_t n_sec = end.tv_sec - start.tv_sec;
  long n_nsec = end.tv_nsec - start.tv_nsec;
  if (n_nsec < 0) {
    --n_sec;
    n_nsec += 1000000000L;
  }
  printf("%d queries in %ld.%09ld\n", OP_NUM, n_sec, n_nsec);
  
#if VERIFY_Q
  puts("========== Read back ===============");
  rc = sqlite3_exec(db, "SELECT * FROM tab", callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
#endif

  sqlite3_close(db);

  fflush(stdout);
  ukplat_halt();
  return 0;
}
