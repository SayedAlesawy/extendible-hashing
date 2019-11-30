#include <map>
#include <vector>
#include <iostream>

using namespace std;

const int BUCKET_MAXSIZE = 3;

struct Bucket
{
  int MAX_SIZE;
  int depth;
  vector<int> data;

  Bucket(int max_size)
  {
    depth = 1;
    MAX_SIZE = max_size;
  }
};

struct Directory
{
  int depth;
  map<int, Bucket *> bucket_map;

  Directory()
  {
    int depth = 1;

    for (int i = 0; i < 2; i++)
    {
      bucket_map[i] = new Bucket(BUCKET_MAXSIZE);
    }
  }
};

Directory directory;

// Assume hash(value) = x LSB bits of value
int hash_key(int x, int value)
{
  int hash = 0;

  for (int i = 0; i < x; i++)
    hash |= (value & (1 << i));

  return hash;
}

void double_dir()
{
  directory.depth++;

  int next = directory.bucket_map.size();

  for (int i = next; i < 2 * next; i++)
  {
    directory.bucket_map[i] = NULL;
  }
}

void redistribute(Bucket *buck1, Bucket *buck2, int buck1_id, int buck2_id)
{
  for (int i = 0; i < buck1->data.size(); i++)
  {
    int hash = hash_key(directory.depth, buck1->data[i]);

    if (hash == buck1_id)
      continue;

    buck2->data.push_back(buck1->data[i]);
    buck1->data.erase(buck1->data.begin() + i);
    i--;
  }
}

Bucket *get_bucket(int key)
{
  for (int i = 0; i < directory.bucket_map.size(); i++)
  {
    if (directory.bucket_map[i] == NULL)
      continue;

    int global_hash = hash_key(directory.bucket_map[i]->depth, key);
    int local_hash = hash_key(directory.bucket_map[i]->depth, i);

    if (global_hash == local_hash)
    {
      return directory.bucket_map[i];
    }
  }

  return NULL;
}

void assign_dangling_ptrs()
{
  for (int i = 0; i < directory.bucket_map.size(); i++)
  {
    if (directory.bucket_map[i] != NULL)
      continue;

    directory.bucket_map[i] = get_bucket(i);
  }
}

void split_bucket(int hash, int value)
{
  int re_hash = hash ^ (1 << directory.depth - 1);
  int hash1 = min(hash, re_hash);
  int hash2 = max(hash, re_hash);

  directory.bucket_map[hash1]->data.push_back(value);
  directory.bucket_map[hash2] = new Bucket(BUCKET_MAXSIZE);
  directory.bucket_map[hash1]->depth++;
  directory.bucket_map[hash2]->depth++;

  redistribute(directory.bucket_map[hash1], directory.bucket_map[hash2], hash1, hash2);
  assign_dangling_ptrs();
}

void insert_in_bucket(int hash, int value)
{
  //Bucket is full
  if (directory.bucket_map[hash]->data.size() == directory.bucket_map[hash]->MAX_SIZE)
  {
    //The directory should double and the bucket should split
    if (directory.bucket_map[hash]->depth == directory.depth)
    {
      double_dir();

      split_bucket(hash, value);
    }
    else
    {
      split_bucket(hash, value);
    }
  }
  else
  {
    directory.bucket_map[hash]->data.push_back(value);
  }
}

void print_directory()
{
  for (int i = 0; i < directory.bucket_map.size(); i++)
  {
    printf("[%d] -> [", i);

    if (directory.bucket_map[i] == NULL)
    {
      printf("]\n");
      continue;
    }

    for (int j = 0; j < directory.bucket_map[i]->data.size(); j++)
    {
      printf("%d", directory.bucket_map[i]->data[j]);

      if (j < directory.bucket_map[i]->data.size() - 1)
        printf(", ");
    }

    printf("]\n");
  }

  printf("---------------------------------------------\n\n");
}

void insert(int value)
{
  int global_depth = directory.depth;

  int hash = hash_key(global_depth, value);

  insert_in_bucket(hash, value);
}

int main()
{
  directory.depth = 1;

  int values[] = {16, 4, 6, 22, 24, 10, 31, 7, 9, 20, 26};
  //int values[] = {4, 6, 2, 12, 9, 11, 7, 15, 13, 8, 12};
  int sz = sizeof(values) / sizeof(values[0]);

  print_directory();

  for (int i = 0; i < sz; i++)
  {
    printf("Inserting %d\n", values[i]);
    insert(values[i]);
    print_directory();
  }
}
