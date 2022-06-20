struct vma {
  struct file* f;
  int off;
  int prot;
  int flag;
  int length;
  uint64 staddr;
  int curoff;
  int curlength;
  uint64 curstaddr;
  int dlength;
  uint64 dstaddr;
  int v;
};

extern struct vma VMA[];
