/*
 *  hangman.c
 *  05/01/01 Original.
 *  04/10/02 Use linked linst instead of array.
 *  04/11/02 Alternative word file.
 *  11/20/03 Added DeleteList
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>

#define MAX_WORD_LEN 20
#define MIN_WORD_LEN 5
#define TRY 6

typedef struct Score {
  int total;
  int won;
  int lost;
} Score;

/*
 * linked list code from http://cslibrary.stanford.edu/105/
 */
typedef struct node {
  char word[MAX_WORD_LEN];
  struct node *next;
} node;

int match(char list[], char c);
void mask(char *word);
int reveal(char *word, char *tword, char c);
int read_words(node **data, char *ifn);
void get_word(char *word, int count, node *data);
void ltrim(char *s);
void chomp(char *s);
int check_word(char *word);
void scoreboard(Score *score);
int play(int word_count, node *words, Score *score);
void RemoveDuplicates(struct node* head);
int Length(struct node* head);
void FrontBackSplit(struct node* source,struct node** frontRef, struct node** backRef);
void MoveNode(struct node** destRef, struct node** sourceRef);
struct node* SortedMerge(struct node* a, struct node* b);
void MergeSort(struct node** headRef);
void GetNth(struct node* head, int index, char *word);
void DeleteList(struct node** headRef);

void DeleteList(struct node** headRef) {
   struct node* current = *headRef; /* deref headRef to get the real head */
   struct node* next;
   while (current != NULL) {
      next = current->next; /* note the next pointer */
      free(current);        /* delete the node */
      current = next;       /* advance to the next node */
   }
   *headRef = NULL;         /* Again, deref headRef to affect
                                the real head back in the caller. */
}

void GetNth(struct node* head, int index, char *word) {
   struct node* current = head;
   int count = 0;

   while (current != NULL) {
      if (count == index) {
        strcpy(word,current->word);
        break;
      }
      count++;
      current = current->next;
   }

   /* assert(0);  if we get to this line, the caller was asking */
              /* for a non-existent element so we assert fail. */
}

void RemoveDuplicates(struct node* head) {
   struct node* current = head;
   if (current == NULL)
     return; /* do nothing if the list is empty */

   /* Compare current node with next node */
   while(current->next!=NULL) {
      if (strcmp(current->word,current->next->word)==0) {
         struct node* nextNext = current->next->next;
         free(current->next);
         current->next = nextNext;
      }
      else {
        current = current->next; /* only advance if no deletion */
      }
   }
}

int Length(struct node* head) {
   int count = 0;
   struct node* current = head;
   while (current != NULL) {
     count++;
     current=current->next;
   }
   return(count);
}

void FrontBackSplit(struct node* source,
struct node** frontRef, struct node** backRef) {
   int len = Length(source);
   int i;
   struct node* current = source;

   if (len < 2) {
      *frontRef = source;
      *backRef = NULL;
   }
   else {
      int hopCount = (len-1)/2;
      for (i = 0; i<hopCount; i++) {
      current = current->next;
   }

   *frontRef = source;
   *backRef = current->next;
   current->next = NULL;
   }
}

void MoveNode(struct node** destRef, struct node** sourceRef) {
  struct node* newNode = *sourceRef; /* the front source node */
   assert(newNode != NULL);
   *sourceRef = newNode->next;       /* Advance the source pointer */
   newNode->next = *destRef;         /* Link the old dest off the new node */
   *destRef = newNode;               /* Move dest to point to the new node */
}

struct node* SortedMerge(struct node* a, struct node* b) {
   struct node* result = NULL;
   struct node** lastPtrRef = &result;  /* point to the last result pointer */
   while (1) {
      if (a==NULL) {
         *lastPtrRef = b;
         break;
      }
      else if (b==NULL) {
         *lastPtrRef = a;
         break;
      }

      if (strcmp(a->word,b->word)<=0) {
         MoveNode(lastPtrRef, &a);
      }
      else {
         MoveNode(lastPtrRef, &b);
      }

      /* advance to point to the next ".next" field */
      lastPtrRef = &((*lastPtrRef)->next);
   }
   return(result);
}

void MergeSort(struct node** headRef) {
   struct node* head = *headRef;
   struct node* a;
   struct node* b;

   /* Base case -- length 0 or 1 */
   if ((head == NULL) || (head->next == NULL)) {
      return;
   }

   /* Split head into 'a' and 'b' sublists */
   FrontBackSplit(head, &a, &b);

   MergeSort(&a);
   MergeSort(&b);
   *headRef = SortedMerge(a, b);
}


void mask(char *word)
{
  char dlm = '_';
  int i, len;
  i=len=0;

  len = strlen(word);

  for(i=0;i<len;i++)
    word[i] = dlm;
}


int reveal(char *word, char *tword, char c)
{
    int len=0;
    int i;
    int success=0;

    len = strlen(word);

    for(i=0;i<len;i++) {
      if (word[i] == c) {
         tword[i] = c;
         success=1;
      }
    }
    return success;
}


int match(char list[], char c)
{
  int i, len, hit;
  i=len=hit=0;

  len = strlen(list);

  if(len == 0)
    return 0;

  for(i=0; i<len; i++) {
    if(c == list[i])
       hit=1;
  }

  return hit;
}


void ltrim(char *s)
{
  /* typecast s as int to keep gcc from complaining */
  char *t = s;
  while(*s) {
    if(isalpha((int)*s))
      break;
    s++;
  }
  strcpy(t,s);
}

void chomp(char *s)
{
    int length=0;
    int i;
    char *t = s;
    length = strlen(s);
    for(i=0;i<=length;i++) {
      if (s[i] == '\n')
        s[i] = '\0';
    }
    strcpy(t,s);
}

int check_word(char *word)
{
   int i;
   int length=0;
   length = strlen(word);

   if(isupper((int)word[0]))
      return 0;

   if(length < MIN_WORD_LEN || length > MAX_WORD_LEN)
      return 0;

   for(i=0;i<length;i++) {
      if(! isalpha((int)word[i]))
         return 0;
   }

   return 1;
}

int read_words(node **data, char *ifn)
{
   FILE *ifp;
   char buf[BUFSIZ];
   int count=0;
   node *current = *data;
   node *head = NULL;
   node *prev = *data;

   if(!(ifp = fopen(ifn, "r")))  {
       fprintf(stderr, "%s %s\n","ERROR: cannot open ", ifn);
       exit (1);
    }

   while (fgets (buf, BUFSIZ, ifp) != NULL) {

       buf[MAX_WORD_LEN] = '\0';

       ltrim(buf);
       chomp(buf);

       if(! check_word(buf))
          continue;

       /* printf("[%s]\n", buf); */

       current = (node*)malloc(sizeof(node));
       if (current == NULL) {
          fprintf(stderr, "%s\n", "ERROR: malloc");
          exit(1);
       }

       if (head == NULL) {
           head = current;
       }
       else {
           prev->next = current;
       }

       current->next = NULL;

       strcpy(current->word, buf);
       prev = current;
   }

    /* rewind the list */
    current = head;

    /* sort */
    /* MergeSort(&current); */

    /* uniq */
    /* RemoveDuplicates(current); */

    count = Length(current);
    printf("Read %d Words\n",count);
    fclose(ifp);

    *data = current;

    return count;
}


void get_word(char *word, int count, struct node *current)
{
   char buf[MAX_WORD_LEN];
   int rnd=0;

   srand48(time(NULL));
   rnd = lrand48() % count;
   GetNth(current,rnd,buf);
   strcpy(word,buf);
}


void scoreboard(Score *score)
{
    float avg=0;
    avg = (double)score->won/ (double)score->total;

    printf("\n=======================================\n");
    printf("Wins   %d\n", score->won);
    printf("Losses %d\n", score->lost);
    printf("Total  %d\n", score->total);
    printf("Avg    %f\n", avg);
    printf("=======================================\n");
}

int play(int word_count, node *words, Score *score)
{
   int try=0;
   int mresult=0;
   int correct=0;
   int pick_index=0;
   char another[MAX_WORD_LEN];
   char guess[MAX_WORD_LEN];
   char word[MAX_WORD_LEN];
   char pick[MAX_WORD_LEN];
   char wrong[MAX_WORD_LEN];
   char tword[MAX_WORD_LEN];

   memset(another,'\0',sizeof(another));
   memset(guess,'\0',sizeof(guess));
   memset(word,'\0',sizeof(word));
   memset(pick,'\0',sizeof(pick));
   memset(wrong,'\0',sizeof(wrong));
   memset(tword,'\0',sizeof(tword));

   score->total++;

   /* get a new random word */
   get_word(word, word_count, words);

   /* make a copy */
   strcpy(tword, word);

   /* hide word */
   mask(tword);

   while(1) {
      printf("Word %s\n", tword);
      printf("[%d]->%s", try, "Guess: ");
      scanf("%s", guess);
      guess[1] = '\0';

      /* See if we already guessed the letter */
      mresult = match(pick, guess[0]);
      if (mresult) {
         printf("You Already picked %c\n",guess[0]);
         continue;
      }
      pick[pick_index] = guess[0];
      pick_index++;

      /* see if we have a correct guess */
      correct = reveal(word, tword, guess[0]);

      /* charge a try if guess is wrong */
      if (!correct) {
         wrong[try] = guess[0];
         wrong[try+1]='\0';
         try++;
      }

      /* Do we have a winner */
      if (strcmp(word,tword)==0) {
          printf("%s\n", word);
          printf("%s\n", "YOU WIN!");
          score->won++;
          break;
      }

      /* A looser */
      if ( try > TRY ) {
         printf("%s\n", "YOU LOOSE :(");
         printf("Word is %s\n", word);
         score->lost++;
         break;
      }

      /* List of wrong guesses */
      if ( wrong[0] > 0 )
         printf("Wrong [%s]\n", wrong);

   }

   /* show statistics */
   scoreboard(score);

   printf("Play Another [Y|N] ? ");
   scanf("%s", another);
   another[1] = '\0';
   another[0] = toupper(another[0]);

   return another[0] == 'Y' ? 1 : 0;
}

int main(int argc, char *argv[]) {
   int word_count=0;
   int more=1;
   int opt=0;
   node *words;
   Score *score;
   char infile[100];
   const char *usage =
   "usage : -[f:h]  \n\
   -f alternate word file, default is /usr/dict/words \n\
   -h help   \n";

   /* default word file */
    memset(infile,'\0',sizeof(infile));
    strcpy(infile, "/usr/dict/words"); 

    /* get options */
    while(1)
    {
      opt = getopt (argc, argv, "f:h");
      if (opt == -1)
        break;

      switch (opt)
        {
        case 'f':
           if (strlen(optarg) > 99) {
              fprintf(stderr, "Filename must be less than 100 chars.\n");
              exit (1);
           }
           strcpy(infile, optarg);
           break;

        case 'h':
          printf("%s", usage);
          exit (0);

        default:
          fprintf(stderr, "%s", usage);
          exit (1);
        } /* end switch */
    }/* end while(1) */

   score = (Score*)malloc(sizeof(Score));
   if (score == NULL) {
      fprintf(stderr, "%s\n", "ERROR: malloc");
      exit(1);
   }

   score->won=0;
   score->lost=0;
   score->total=0;

   /* read the list of words */
   printf("Reading %s\n", infile);
   word_count = read_words(&words, infile);
   if (word_count == 0){
      fprintf(stderr, "%s ERROR: There are no valid words in %s\n",argv[0], infile);
      fprintf(stderr, "\tA valid word is >= %d & <= %d letters\n",
            MIN_WORD_LEN, MAX_WORD_LEN);
      fprintf(stderr, "\tNon-capitalized, containing only letters.\n");
      exit (1);
   }

   /* play a game */
   while(1) {
       more = play(word_count, words, score);
       if (more != 1)
          break;
   }

   DeleteList(&words);
   free(score);
   return 0;
}

/* -------------- End Hangman --------------- */

