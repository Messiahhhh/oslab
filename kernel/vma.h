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
  int refcnt;
  int unmlength;
  int v;
};

extern struct vma VMA[];
