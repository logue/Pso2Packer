#pragma once

//--------------------------------------------------
// BlowFish class
//--------------------------------------------------
class clBlowFish {
public:
  clBlowFish(void *key, int size, bool swap = true);

  // Encryption / Decryption
  void Encrypt(void *data, int size);
  void Decrypt(void *data, int size);

private:
  unsigned int _table1[18];
  unsigned int _table2[4][0x100];
  void Encrypt(unsigned int *v1, unsigned int *v2);
  void Decrypt(unsigned int *v1, unsigned int *v2);
  inline unsigned int BlowFish(unsigned int v, unsigned int i);
};
