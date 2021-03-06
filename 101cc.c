#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
  TK_NUM = 256, // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合、その数値
  char *input; // トークン文字列（エラーメッセージ用）
} Token;

enum {
  ND_NUM = 256, // 整数のノード型
};

typedef struct Node {
  int ty;           // 演算子かND_NUM
  struct Node *lhs; //  左辺
  struct Node *rhs; //  右辺
  int val;          //  tyがND_NUMの場合のみ使う
} Node;

Node *new_node();
Node *new_node_num();
Node *expr();
Node *mul();
Node *term();
void error(char *, char *);
void gen(Node *);

// トークナイズした結果のトークン列はこの配列に保存する
// １００個以上のトークンは来ないものとする
Token tokens[100];

int pos;

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p =='(' || *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      //fprintf(stderr, "%s\n", p);
      i++;
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      //fprintf(stderr, "%s\n", p);
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "トークナイズできません：%s\n", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

Node *new_node(int op, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *expr() {
  Node *lhs = mul();

  //fprintf(stderr, "expr\n");

  if (tokens[pos].ty == '+') {
    pos++;
    return new_node('+', lhs, expr());
  }

  if (tokens[pos].ty == '-') {
    pos++;
    return new_node('-', lhs, expr());
  }

  return lhs;
}

Node *mul() {
  Node *lhs = term();

  //fprintf(stderr, "mul\n");

  if (tokens[pos].ty == '*') {
    pos++;
    return new_node('*', lhs, mul());
  }

  if (tokens[pos].ty == '/') {
    pos++;
    return new_node('/', lhs, mul());
  }

  return lhs;
}

Node *term() {
  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  //fprintf(stderr, "term\n");

  if (tokens[pos].ty == '(') {
    pos++;
    Node *node = expr();

    /*
    if (tokens[pos].ty == ')')
      error("開きカッコに対応する閉じカッコがありません： %s",
            tokens[pos].input); i
     */

    pos++;

    return node;
  }

  error("数値でも開きカッコでもないトークンです： %s", tokens[pos].input);

  return (Node *)NULL;
}

// エラーを報告するための関数
void error(char *m, char *p) {
  fprintf(stderr, m, p);
  exit(1);
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズする
  tokenize(argv[1]);

  Node *node = expr();

  //    アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global _main\n");
  printf("_main:\n");

  //    抽象構文木を下りながらコードを生成
  gen(node);

  //    スタックトップに式全体の値が残っているはずなので
  //    それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;

}
