#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

enum {not_end, end_without_spaces, end_with_extraspaces, quit, unblcd_quotes, 
      sym_with_extraspaces, extra_files};
enum {notf, readf, wrt_to_beg, wrt_to_end};


struct list_letters
{
  char letter;
  struct list_letters *next;
};

struct list_words
{
  char *word;
  struct list_words *next;
};

struct list_arr
{
  char **arr_str;
  struct list_arr *next;
};

struct spec_sym_arr
{
  int type_out;
  int bg;
  int conv;
  char *f_in;
  char *f_out;
  struct list_arr *list_proc; 
};

struct pipes_pids
{
  int pipe[2];
  int pid_in, pid_out;
  struct pipes_pids *next;
};

void null_struc(struct spec_sym_arr *struc_spec)
{
  struc_spec->type_out=wrt_to_beg;
  struc_spec->bg=0;
  struc_spec->conv=0;
  struc_spec->f_in=NULL;
  struc_spec->f_out=NULL;
  struc_spec->list_proc=NULL;
}

void free_with_null(char *p)
{
  if (p) {
    free(p);
  }
}

void print_inv()
{
  printf("Type command(for exit ^D):");
}

void fork_err(int pid)
{
  if (pid==-1) {
    perror("fork");
  }
}

void printstring(char *beg)
{
  int i=0;
  while (beg[i]) {
    putchar(beg[i]);
    i++;
  }
  printf("\n");
}

void printwords(struct list_words *first)
{
  while (first) {
    printstring(first->word);
    first=first->next;
  }
}

void print_list_of_let(struct list_letters *first)
{
  while (first) {
    putchar(first->letter);
    first=first->next;
  }
}

void print_proc(struct list_arr *first)
{
  int i;
  struct list_arr *p;
  while (first) {
    p=first->next;
    i=0;
    while (first->arr_str[i]) {
      printf("%s\n",first->arr_str[i]);
      i++;
    }
    printf("!!\n");
    first=p;
  }
}

struct list_letters *delete_let_list(struct list_letters *first)
{
  struct list_letters *cur;
  while (first) {
    cur=first->next;
    free(first);
    first=cur;
  }
  return NULL;
}

void delete_wrd_list(struct list_words **first)
{
  struct list_words *p;
  while (*first) {
    p=(*first)->next;
    free(*first);
    *first=p;
  }
}

void delete_arr_list(struct list_arr **first)
{
  struct list_arr *p;
  int i;
  while (*first) {
    p=(*first)->next;
    i=0;
    while ((**first).arr_str[i]) {
      free((**first).arr_str[i]);
      i++;
    }
    free((**first).arr_str);
    free(*first);
    *first=p;
  }
}

void delete_pipes(struct pipes_pids **first)
{
  struct pipes_pids *p;
  while (*first) {
    p=(*first)->next;
    free(*first);
    *first=p;
  }
}

int count_proc(struct list_arr *first)
{
  int i=0;
  while (first) {
    i++;
    first=first->next;
  }
  return i;
}

int count_wrds_list(struct list_words *first)
{
  int cnt=0;
  while (first) {
    cnt++;
    first=first->next;
  }
  return cnt;

}

int count_let(struct list_letters *first)
{
  int cnt=0;
  while (first) {
    cnt++;
    first=first->next;
  }
  return cnt;
}

int count_wrds_arr(char **arr_of_str)
{
  int i=0;
  while (arr_of_str[i]) {
    i++;
  }
  return i;
}

char *create_move_str(struct list_letters *first_ltr)
{
  int i=0;
  char *beg_str;
  beg_str=malloc( sizeof(*beg_str)*(count_let(first_ltr)+1) ); 
  while (first_ltr) {
    beg_str[i]=first_ltr->letter;
    first_ltr=first_ltr->next;
    i++;
  }
  beg_str[i]=0;
  return beg_str;
}

int cmpstrings(char *firststr, char *secondstr)
{
  int check=1,i=0;
  while ((check) && (firststr[i]!=0) && (secondstr[i]!=0)) {
    if (firststr[i]!=secondstr[i]) {
      check=0;
      break;
    }
    i++;
  }
  if (firststr[i]==secondstr[i]) { 
    return 1;
  }
  else {
    return 0;
  }
}

char handle_first
(struct spec_sym_arr *struc_spec, int *check_end, int *check_f_cur)
{
  char tmp;
  tmp=getchar();
  if ((*check_f_cur==wrt_to_beg) && (tmp=='>')) {
    *check_f_cur=wrt_to_end;
  }
  if (tmp==' ') {
    while ((tmp=getchar())==' ')
    {}
  }
  if (tmp=='|') {
    struc_spec->conv=1;
    tmp='\n';
  }
  if (tmp=='&') {
    struc_spec->bg=1;
    tmp='\n';
  }
  if (tmp=='\n') {
    *check_end=end_with_extraspaces;
  }
  if ((tmp=='>') && ((*check_f_cur)!=wrt_to_end)) { 
      *check_f_cur=wrt_to_beg;
  }
  if (tmp!='>') {
    *check_f_cur=notf;
  }
  if ((tmp=='>') || (tmp=='<')) {
    *check_end=sym_with_extraspaces;
  }
  return tmp;
}

void handle_last(char tmp, int *check_end, int check_q, int *check_f_cur)
{
  if (tmp==EOF) {
    *check_end=quit;
  }
  if ((tmp=='\n') && ((*check_end)!=end_with_extraspaces)) {
    *check_end=end_without_spaces;
  }
  if (check_q) {
    *check_end=unblcd_quotes;
  }
  if (tmp=='<') {
    *check_f_cur=readf;
  }
  if ((tmp=='>') && ((*check_f_cur)==notf)) {
    *check_f_cur=wrt_to_beg;
  }
}

int check_not_end(char tmp)
{
  return ((tmp!=EOF) && (tmp!='\n'));
}

int check_in_quot(char tmp, int check_q)
{
  return (((tmp!='<') && (tmp!='>') && (tmp!=' ')) || (check_q));
}

struct list_letters **
add_let(struct list_letters **p, char *tmp)
{
  (*p)=malloc(sizeof(**p));
  (*p)->letter=*tmp;
  (*p)->next=NULL;
  p=&((*p)->next);
  *tmp=getchar();
  return p;
}

char handle_middle(struct spec_sym_arr *struc_spec, int *check_q, char tmp)
{
  if (tmp=='"') {
    *check_q=!(*check_q);
    return getchar();
  } 
  if ( (tmp=='|') && (!(*check_q)) ) {
    struc_spec->conv=1;
    return '\n';
  }
  if ((tmp=='&') && (!*check_q)) {
    struc_spec->bg=1;
    return '\n';
  }
  return tmp;
}

struct list_letters *
readword(struct spec_sym_arr *struc_spec, int *check_end, int *check_f_cur)
{
  char tmp;
  int check_q=0;
  struct list_letters *first=NULL,**p;
  *check_end=not_end;

  p=&first;
  tmp=handle_first(struc_spec, check_end, check_f_cur);
  while (check_not_end(tmp) && check_in_quot(tmp, check_q) ) {
    tmp=handle_middle(struc_spec, &check_q, tmp);
    if (check_not_end(tmp) && (tmp!='"') && ((tmp!=' ') || (check_q))) {
      p=add_let(p,&tmp);
    }
    *check_f_cur=notf;
  }
  handle_last(tmp, check_end, check_q, check_f_cur);
  return first;
}

char **arr_words(struct list_words *first_list)
{
  int i=0, emount_wrds;
  char **first_arr;
  emount_wrds=count_wrds_list(first_list);
  first_arr=malloc((sizeof(*first_arr))*(emount_wrds+1));
  while (first_list) {
    first_arr[i]=first_list->word;
    i++;
    first_list=first_list->next;
  }
  first_arr[i]=NULL;
  return first_arr;
}

/* a - check_f_cur; b - check_f_prev */
int change_type_out(struct spec_sym_arr *struc_spec, int a, int b)
{
  if (a==wrt_to_end) {
    return wrt_to_end;
  } 
  if ((b==wrt_to_beg) && (a==notf)) {
    return wrt_to_beg;
  }
  return struc_spec->type_out;
} 

struct list_words **add_wrd(struct list_words **p, struct list_letters *first)
{
  (*p)=malloc(sizeof(**p));
  (*p)->next=NULL;
  (*p)->word=create_move_str(first);
  p=&((*p)->next);
  return p;
}

void printerr(int check_end)
{
  if (check_end==unblcd_quotes) {
    printf("Unbalanced quotes!\n");
  }
  if (check_end==extra_files) {
    printf("Extra files!\n");
  }
}

int check_extr_f
(struct spec_sym_arr *struc_spec, int check_f_prev, int check_end)
{
  if (check_f_prev==readf) {
    if (struc_spec->f_in) {
      check_end=extra_files;
    }
  }
  if ((check_f_prev==wrt_to_beg) || (check_f_prev==wrt_to_end)) {
    if (struc_spec->f_out) {
      check_end=extra_files;
    }
  }
  return check_end;
}


void write_name_f(struct spec_sym_arr *struc_spec, int check_f_prev, 
                  struct list_letters *firstletter)
{
  if (check_f_prev==readf) {
    struc_spec->f_in=create_move_str(firstletter);
  }
  if ((check_f_prev==wrt_to_beg) || (check_f_prev==wrt_to_end)) {
    struc_spec->f_out=create_move_str(firstletter);
  }
}

struct list_words *
create_wrds_list(struct spec_sym_arr *struc_spec, int *check_err)
{
  struct list_words *firstwrd=NULL,**p;
  struct list_letters *firstletter;
  int check_end=not_end, check_f_cur=notf, check_f_prev=notf;

  p=&firstwrd;
  while ((check_end==not_end) || (check_end==sym_with_extraspaces)) {
    check_f_prev=check_f_cur;
    firstletter=readword(struc_spec, &check_end, &check_f_cur);
    if ((check_end==end_without_spaces) || (check_end==not_end)) {
      check_end=check_extr_f(struc_spec, check_f_prev, check_end);
      if (check_end!=extra_files) {
        write_name_f(struc_spec, check_f_prev, firstletter);
      }
      if (check_f_prev==notf) {
        p=add_wrd(p, firstletter);
        firstletter=delete_let_list(firstletter);
      }
    }
    struc_spec->type_out=change_type_out(struc_spec, check_f_cur, check_f_prev);
  }
  firstletter=delete_let_list(firstletter);
  *check_err=check_end;
  printerr(check_end);
  return firstwrd;
}

void handle_cd(char **arr_of_str)
{
  int help, emount_wrds;
  emount_wrds=count_wrds_arr(arr_of_str);
  if (emount_wrds==2) {
    help=chdir(arr_of_str[1]);
    if (help==-1) {
      perror(arr_of_str[1]);
    }
    print_inv();
  }
  else {
    printf("you have to put 1 name of directory\n");
    print_inv();
  }
}

void open_fd_in(struct spec_sym_arr *struc_spec)
{
  int fdin;
  if (struc_spec->f_in) {
    fdin=open(struc_spec->f_in, O_RDONLY);
    if (fdin==-1) {
      perror(struc_spec->f_in);
    }
    else {
      dup2(fdin,0);
      close(fdin);
    }
  }
}

void open_fd_out(struct spec_sym_arr *struc_spec)
{
  int fdout;
  if (struc_spec->f_out) {
    if (struc_spec->type_out==2) {
      fdout=open(struc_spec->f_out, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    }
    else {
      fdout=open(struc_spec->f_out, O_WRONLY|O_CREAT|O_APPEND, 0666);
    }
    if (fdout==-1) {
      perror(struc_spec->f_out);
    }
    else {
      dup2(fdout,1);
      close(fdout);
    }
  }
}

void add_pipe(struct pipes_pids **p)
{
  (*p)=malloc(sizeof(**p));
  pipe((**p).pipe);
  (**p).next=NULL;
}

void close_extr_pipes(struct pipes_pids *first_pipe, int which_proc)
{
  int i=1;
  while (first_pipe) {
    if (i==which_proc-1) {
      dup2(first_pipe->pipe[0],0);
    } 
    close(first_pipe->pipe[0]);
    if (i==which_proc) {
      dup2(first_pipe->pipe[1],1);
    }
    close(first_pipe->pipe[1]);
    i++;
    first_pipe=first_pipe->next;
  }
}

int req_proc(int emnt_proc, int killed_pid, struct pipes_pids *first_pipe)
{
  if (killed_pid==first_pipe->pid_out) {
    return emnt_proc-1;
  }
  while (first_pipe) {
    if (killed_pid==first_pipe->pid_in) {
      return emnt_proc-1;
    }
    first_pipe=first_pipe->next;
  }
  return emnt_proc;
}

void create_all_pipes(struct pipes_pids **p, int emnt_proc)
{ 
  int i;
  for (i=1;i<emnt_proc;i++) {
    add_pipe(p);
    p=&((**p).next);
  }
}

int handle_one_proc(struct list_arr *first_arr, struct spec_sym_arr *struc_spec)
{
  int pid_if_one;
  pid_if_one=fork();
  switch (pid_if_one) {
    case 0:
      open_fd_in(struc_spec);
      open_fd_out(struc_spec);
      execvp((*first_arr).arr_str[0], (*first_arr).arr_str);
      perror((*first_arr).arr_str[0]);
      exit(1);
      break;
    case -1:
      fork_err(pid_if_one);
      break;
  }
  return pid_if_one;
}

void run_proc(struct list_arr *first_arr, struct pipes_pids *first_pipe, 
              int which_proc)
{
  close_extr_pipes(first_pipe, which_proc);
  execvp((*first_arr).arr_str[0], (*first_arr).arr_str);
  perror((*first_arr).arr_str[0]);
  exit(1);
}

struct pipes_pids **run_mid_proc(struct list_arr **first_arr, 
                                 struct pipes_pids **p)
{
  struct pipes_pids *first_pipe;
  int i, emnt_proc;

  emnt_proc=count_proc(*first_arr)+1;
  first_pipe=*p;
  i=2;
  while (i<emnt_proc) {
    (**p).pid_in=fork();
    fork_err((**p).pid_in);
    if (!(**p).pid_in) {
      run_proc(*first_arr, first_pipe, i);
    }
    (**p).next->pid_out=(**p).pid_in;
    p=&((**p).next);
    i++;
    *first_arr=(*first_arr)->next;
  }
  return p;
}

void handle_several_proc(struct list_arr *first_arr, struct pipes_pids **p, 
                         struct spec_sym_arr *struc_spec)
{
  int emnt_proc;
  struct pipes_pids *first_pipe;
  first_pipe=*p;
  emnt_proc=count_proc(first_arr);
  first_pipe->pid_out=fork();
  fork_err(first_pipe->pid_out);
  if (!first_pipe->pid_out) {
    open_fd_in(struc_spec);
    run_proc(first_arr, first_pipe, 1);
  }
  first_arr=first_arr->next;
  p=run_mid_proc(&first_arr, p);
  if (emnt_proc!=1) {
    (**p).pid_in=fork();
    fork_err((**p).pid_in);
    if (!(**p).pid_in) {
      open_fd_out(struc_spec);
      run_proc(first_arr, first_pipe, emnt_proc);
    }
  }
}

void wait_for_it(struct spec_sym_arr *struc_spec, int pid_if_one,
                 struct pipes_pids *first_pipe)
{
  int killed_pid, emnt_proc;
  emnt_proc=count_proc(struc_spec->list_proc);
  if (!(struc_spec->bg)) {
    if (emnt_proc==1) {
      while (wait(NULL)!=pid_if_one)
      {}
    }
    else {
      while (emnt_proc!=0) {
        killed_pid=wait(NULL);
        emnt_proc=req_proc(emnt_proc, killed_pid, first_pipe); 
      }
    }
    print_inv();
  }
  else {
    while (wait4(-1, NULL, WNOHANG, NULL)>0)
    {}
    struc_spec->bg=0;
  }
}

void handle_conv(struct list_arr *first_arr, struct spec_sym_arr *struc_spec)
{
  int emnt_proc, pid_if_one;
  struct pipes_pids *first_pipe=NULL, **p;
  emnt_proc=count_proc(first_arr);
  p=&first_pipe;
  create_all_pipes(p, emnt_proc);
  if (emnt_proc==1) {
    pid_if_one=handle_one_proc(first_arr, struc_spec);
  }
  else {
    handle_several_proc(first_arr, p, struc_spec);
  }
  close_extr_pipes(first_pipe, 0);
  wait_for_it(struc_spec, pid_if_one, first_pipe);
  delete_pipes(&first_pipe);
}

void exec_with_cd(struct spec_sym_arr *struc_spec)
{
  if (cmpstrings("cd", (((*struc_spec).list_proc)->arr_str)[0])) {
    handle_cd(((*struc_spec).list_proc)->arr_str);
  }
  else {
    handle_conv(struc_spec->list_proc, struc_spec);
  }
}

struct list_arr **add_proc(struct list_arr **p, struct list_words *firstwrd,
                           struct spec_sym_arr *struc_spec)
{
  (*p)=malloc(sizeof(**p));
  (**p).next=NULL;
  (**p).arr_str=arr_words(firstwrd);
  if (!struc_spec->conv) {
    exec_with_cd(struc_spec);
    delete_arr_list(&struc_spec->list_proc);  
    p=&(struc_spec->list_proc);
    free_with_null(struc_spec->f_in);
    free_with_null(struc_spec->f_out);
    null_struc(struc_spec); 
  }
  else {
    struc_spec->conv=0;
    p=&((**p).next);
  }
  return p;
}

int main()
{
  int check_err=not_end;
  struct list_words *firstwrd=NULL;
  struct spec_sym_arr struc_spec;
  struct list_arr **p;
  print_inv();
  p=&(struc_spec.list_proc);
  null_struc(&struc_spec);
  while (check_err!=quit) {
    while (wait4(-1, NULL, WNOHANG, NULL)>0)
    {}
    firstwrd=create_wrds_list(&struc_spec, &check_err);
    if ((check_err!=quit) && (check_err!=unblcd_quotes) && (firstwrd) && 
        (check_err!=extra_files)) {
      p=add_proc(p, firstwrd, &struc_spec);
    }
    if (((!firstwrd) && (check_err!=quit)) || (check_err==extra_files)) {
      print_inv();
    }
    delete_wrd_list(&firstwrd);
  }
  putchar('\n');
  return 0;
}
