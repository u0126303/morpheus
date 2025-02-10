int test(int a, int b)
{
  extern char *p;
  int result = 10;

  if (a > b) {
    int v = b + 2;
    int w = p[v];
    result += w;
  }
  else
  {
    int v = 3 + a;
    result += v;
  }

  return result;
}
