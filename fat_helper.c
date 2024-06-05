#include "fat_helper.h"

int GLOBAL_FAT_DEVICE_SECTORS_IN_RAM = 0;
unsigned int read_bytes_to_int(FILE *fp, int *cursor_loc, int offset_start,
                               int offset, int size) {
  if (size > 4) {
    fprintf(stderr,
            "Size missmatch reading bytes at cursor_loc=%d offset_start=%d "
            "offset=%d size=%d\n",
            *cursor_loc, offset_start, offset, size);
    exit(1);
  }
  int result = 0;
  while (*cursor_loc < (offset_start + offset)) {
    getc(fp);
    *cursor_loc = *cursor_loc + 1;
  }

  for (int i = 0; i < size; i++) {
    result += (unsigned char)getc(fp) << (i * 8);
    *cursor_loc = *cursor_loc + 1;
  }
  return result;
}
void read_bytes_to_char_arr(FILE *fp, unsigned char *source, int *cursor,
                            int offset_start, int offset, int size) {
  printf("Cursor pre %d, ", *cursor);
  while (*cursor < (offset_start + offset)) {
    getc(fp);
    *cursor = *cursor + 1;
  }
  printf("Cursor loc %d ofset %d, offset_start %d\n", *cursor, offset,
         offset_start);
  for (int i = 0; i < size; i++) {
    source[i] = getc(fp);
    putchar(source[i]);
    *cursor = *cursor + 1;
  }
}

int FAT_Device_identify_fat_type_and_init(FAT_Device *d, int *c, FILE *fp) {
  // Calculate RootDirSectors
  long RootDirSectors;
  RootDirSectors =
      ((d->BPB_RootEntCnt * 32) + (d->BPB_BytsPerSec - 1)) / d->BPB_BytsPerSec;
  d->RootDirSectors = RootDirSectors;

  // Calculate DataSec
  long FATSz, TotSec, DataSec;
  if (d->BPB_FATSz16 != 0) {
    // init for fat 16 and 12 since they share the same types of data
    FAT_16_or_12_Device device;
    device.BS_DrvNum = read_bytes_to_int(fp, c, 0, 36, 1);
    device.BS_Reserved1 = read_bytes_to_int(fp, c, 0, 37, 1);
    device.BS_BootSig = read_bytes_to_int(fp, c, 0, 38, 1);
    device.BS_VolID = read_bytes_to_int(fp, c, 0, 39, 4);
    read_bytes_to_char_arr(fp, device.BS_VolLab, c, 0, 43, 11);
    read_bytes_to_char_arr(fp, device.BS_FilSysType, c, 0, 54, 8);
    FAT_Type ft;
    ft.FAT_16_or_12 = device;
    d->FAT_Type_Specifics = ft;
    FATSz = d->BPB_FATSz16;
    d->FATSz = FATSz;
  } else {
    // init for fat 32  since they share the same types of data
    FAT_32_Device device;
    device.BPB_FATSz32 = read_bytes_to_int(fp, c, 0, 36, 4);
    device.BPB_ExtFlags = read_bytes_to_int(fp, c, 0, 40, 2);
    device.BPB_FSVer = read_bytes_to_int(fp, c, 0, 42, 2);
    device.BPB_RootClus = read_bytes_to_int(fp, c, 0, 44, 4);
    device.BPB_FSInfo = read_bytes_to_int(fp, c, 0, 48, 2);
    device.BPB_BkBootSec = read_bytes_to_int(fp, c, 0, 50, 2);
    read_bytes_to_char_arr(fp, device.BPB_Reserved, c, 0, 52, 12);
    // device.BPB_Reserved = read_bytes_to_int(fp, c, 0, 52, 12);
    device.BS_DrvNum = read_bytes_to_int(fp, c, 0, 64, 1);
    device.BS_Reserved1 = read_bytes_to_int(fp, c, 0, 65, 1);
    device.BS_BootSig = read_bytes_to_int(fp, c, 0, 66, 1);
    device.BS_VolID = read_bytes_to_int(fp, c, 0, 67, 1);
    read_bytes_to_char_arr(fp, device.BS_VolLab, c, 0, 71, 11);
    read_bytes_to_char_arr(fp, device.BS_FilSysType, c, 0, 82, 8);
    FAT_Type ft;
    ft.FAT_32 = device;
    d->FAT_Type_Specifics = ft;
    FATSz = d->FAT_Type_Specifics.FAT_32.BPB_FATSz32;
    d->FATSz = FATSz;
  }

  if (d->BPB_TotSec16 != 0) {
    TotSec = d->BPB_TotSec16;
  } else {
    TotSec = d->BPB_TotSec32;
  }

  DataSec =
      TotSec - (d->BPB_RsvdSecCnt + (d->BPB_NumFATs * FATSz) + RootDirSectors);

  // Calculate CountofClusters
  int CountofClusters = DataSec / d->BPB_SecPerClus;

  // Determine the FAT type based on the cluster count
  if (CountofClusters < 4085) {
    return 12; // Volume is FAT12
  } else if (CountofClusters < 65525) {
    return 16; // Volume is FAT16
  } else {
    return 32; // Volume is FAT32
  }
}
FAT_Device FAT_Device_init(FILE *fp) {
  int cursor = 0;
  FAT_Device d;


  // Read and initialize the structure fields from the binary file
  read_bytes_to_char_arr(fp, d.BS_jmpBoot, &cursor, 0, 0, 3);
  read_bytes_to_char_arr(fp, d.BS_OEMName, &cursor, 0, 3, 8);
  d.BPB_BytsPerSec = read_bytes_to_int(fp, &cursor, 0, 11, 2);

  // Initialize more fields based on the provided table
  d.BPB_SecPerClus = read_bytes_to_int(fp, &cursor, 0, 13, 1);
  d.BPB_RsvdSecCnt = read_bytes_to_int(fp, &cursor, 0, 14, 2);
  d.BPB_NumFATs = read_bytes_to_int(fp, &cursor, 0, 16, 1);
  d.BPB_RootEntCnt = read_bytes_to_int(fp, &cursor, 0, 17, 2);
  d.BPB_TotSec16 = read_bytes_to_int(fp, &cursor, 0, 19, 2);
  d.BPB_Media = read_bytes_to_int(fp, &cursor, 0, 21, 1);
  d.BPB_FATSz16 = read_bytes_to_int(fp, &cursor, 0, 22, 2);
  d.BPB_SecPerTrk = read_bytes_to_int(fp, &cursor, 0, 24, 2);
  d.BPB_NumHeads = read_bytes_to_int(fp, &cursor, 0, 26, 2);
  d.BPB_HiddSec = read_bytes_to_int(fp, &cursor, 0, 28, 4);
  d.BPB_TotSec32 = read_bytes_to_int(fp, &cursor, 0, 32, 4);

  // Set the FAT_Type_Val based on your logic (e.g., identify FAT12, FAT16, or
  // FAT32)
  d.FAT_Type_Val = FAT_Device_identify_fat_type_and_init(&d, &cursor, fp);

  d.FirstDataSector =  d.BPB_RsvdSecCnt + (d.BPB_NumFATs * d.FATSz) + d.RootDirSectors;
  if (GLOBAL_FAT_DEVICE_SECTORS_IN_RAM) {
    if (d.FAT_Type_Val == 32) {
      d.sectors = (FAT_Sector *)malloc(sizeof(FAT_Sector *) * d.BPB_TotSec32);
      for(int i = 0; i < d.BPB_TotSec32; i++) {
        d.sectors[i].data = NULL;
        d.sectors[i].id = i;
        d.sectors[i].ptr_valid = 0;
      }
      // TODO FINISH THIS
      // TODO ALLOW FOR FLAGGING WETHER OR NOT TO STORE SECTORS IN RAM
    } else {
      d.sectors = (FAT_Sector *)malloc(sizeof(FAT_Sector *) * d.BPB_TotSec16);
        for(int i = 0; i < d.BPB_TotSec16; i++) {
            d.sectors[i].data = NULL;
            d.sectors[i].id = i;
            d.sectors[i].ptr_valid = 0;
        }
      // TODO FINISH THIS
      // TODO ALLOW FOR FLAGGING WETHER OR NOT TO STORE SECTORS IN RAM
    }
  }

  printf("FAT TYPE = %ld\n", d.FAT_Type_Val);
  printf("FAT #sectors %d\n", d.BPB_TotSec16);
  printf("RESERVED SECTORS %d \n", d.BPB_RsvdSecCnt);
  printf("NUM FATS %ld \n", d.BPB_NumFATs);
  printf("FATZ %ld\n", d.FATSz);
  printf("ROOT DIR SECTOR %ld \n", d.RootDirSectors);
  printf("First data sector %ld\n", d.FirstDataSector);
  printf("FILENAME %s\n", d.BS_OEMName);
  printf("BYTES PER SEC %d\n", d.BPB_BytsPerSec);
  printf("SEC PER CLUSTER %d\n", d.BPB_SecPerClus);

    // get the root directory
    // FirstRootDirSecNum = BPB_ResvdSecCnt + (BPB_NumFATs * BPB_FATSz16)

    long FirstRootDirSecNum = d.BPB_RsvdSecCnt + (d.BPB_NumFATs * d.BPB_FATSz16);
    printf("FirstRootDirSecNum %ld\n", FirstRootDirSecNum);
    // RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec;

    printf("FIRST\n");
    FAT_Device_get_dir(&d, fp, FirstRootDirSecNum, 0);
    printf("SECOND\n");
    FAT_Device_get_dir(&d, fp, FirstRootDirSecNum, 1);
    printf("THIRD\n");
    FAT_Device_get_dir(&d, fp, FirstRootDirSecNum, 2);

    exit(0); // TODO remove this exit lmao
  // Based on the FAT type, initialize the appropriate FAT_Type struct inside
  // the union
  // TODO
  return d;
}

int FAT_Device_first_sector_of_cluster(FAT_Device *d, int n) {
  return ((n - 2) * d->BPB_SecPerClus) + d->FirstDataSector;
}

int FAT_Device_get_cluster_sector_number(FAT_Device *d, int cluster_number) {
  if (d->FAT_Type_Val == 16) {
    int FATOffset = cluster_number * 2;
    return d->BPB_RsvdSecCnt + (FATOffset / d->BPB_BytsPerSec);
  }
  if (d->FAT_Type_Val == 32) {
    int FATOffset = cluster_number * 4;
    return d->BPB_RsvdSecCnt + (FATOffset / d->BPB_BytsPerSec);
  }
  int FATOffset = cluster_number + (cluster_number / 2);
  return d->BPB_RsvdSecCnt + (FATOffset / d->BPB_BytsPerSec);
}

int FAT_Device_get_cluster_entry_offset(FAT_Device *d, int cluster_number) {
  if (d->FAT_Type_Val == 16) {
    int FATOffset = cluster_number * 2;
    return FATOffset - (d->BPB_BytsPerSec * (FATOffset / d->BPB_BytsPerSec));
  }
  if (d->FAT_Type_Val == 32) {
    int FATOffset = cluster_number * 4;
    return FATOffset - (d->BPB_BytsPerSec * (FATOffset / d->BPB_BytsPerSec));
  }
  int FATOffset = cluster_number + (cluster_number / 2);
  return FATOffset - (d->BPB_BytsPerSec * (FATOffset / d->BPB_BytsPerSec));
}

void FAT_Device_load_sector(FAT_Device *d, FILE *fp, int sector_num) {
  // get the sector
  // TODO: recommended to load two sectors on FAT12
  if (d->sectors[sector_num].ptr_valid != 0) {
    // better not leak some memory!
    free(d->sectors[sector_num].data);
    d->sectors[sector_num].data = (unsigned char *)malloc(sizeof(char) * d->BPB_BytsPerSec);
    if (d->sectors[sector_num].id != sector_num) {
      printf("ERROR, sector does not alight with BPB data\n");
      exit(0);
    }

  } else {
    FAT_Sector s;
    s.id = sector_num;
    s.data = (unsigned char *)malloc(sizeof(char) * d->BPB_BytsPerSec);
    s.ptr_valid = 1;
    d->sectors[sector_num] = s;
  }

  // might as well just read it now from file istead of setting to zeros
  // TODO implement global reading functions rather than some FSEEKs and such
  fseek(fp, sector_num * d->BPB_BytsPerSec, SEEK_SET);

  for (int i = 0; i < d->BPB_BytsPerSec; i++) {
    d->sectors[sector_num].data[i] = getc(fp);
  }
  return;
}

unsigned short FAT_Device_12_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
  if (d->FAT_Type_Val != 12) {
    printf("wrong fat entry routine called\n");
    exit(1);
  }
  int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
  if (!(d->sectors[sector_number].data)) {
    FAT_Device_load_sector(d, fp, sector_number);
  }
  int sector_offset = FAT_Device_get_cluster_entry_offset(d, cluster_number);
  unsigned short cluster_first_entry =
      *((u16 *)&d->sectors[sector_number].data[sector_offset]);
  if (cluster_first_entry & 0x1) {
    return cluster_first_entry >> 4;
  } else {
    return cluster_first_entry & 0x0FFF;
  }
}

unsigned int FAT_Device_32_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
  if (d->FAT_Type_Val != 32) {
    printf("wrong fat entry routine called\n");
    exit(1);
  }
  int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
  if (!(d->sectors[sector_number].data)) {
    FAT_Device_load_sector(d, fp, sector_number);
  }
  int sector_offset = FAT_Device_get_cluster_entry_offset(d, cluster_number);

  return *((u32 *)&d->sectors[sector_number].data[sector_offset]) & 0x0FFFFFFF;
}

unsigned short FAT_Device_16_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
  if (d->FAT_Type_Val != 16) {
    printf("wrong fat entry routine called\n");
    exit(1);
  }
  int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
  if (!(d->sectors[sector_number].data)) {
    FAT_Device_load_sector(d, fp, sector_number);
  }
  int sector_offset = FAT_Device_get_cluster_entry_offset(d, cluster_number);

  return *((u16 *)&d->sectors[sector_number].data[sector_offset]);
}

unsigned int FAT_Device_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
  if (d->FAT_Type_Val == 12) {
    return FAT_Device_12_get_cluster_entry_val(d, fp, cluster_number);
  }
  if (d->FAT_Type_Val == 16) {
    return FAT_Device_16_get_cluster_entry_val(d, fp, cluster_number);
  }
  if (d->FAT_Type_Val == 32) {
    return FAT_Device_32_get_cluster_entry_val(d, fp, cluster_number);
  }
  printf("FAT TYPE NOT RECOGNIZED\n");
  exit(1);
}

void FAT_Device_Sector_read_to_char_arr(FAT_Device *d, FILE *fp, int sector_number, unsigned char *source, unsigned int start_offset, unsigned int offset, unsigned int size) {
    if (!(d->sectors[sector_number].ptr_valid)) {
        FAT_Device_load_sector(d, fp, sector_number);
    }
    for (int i = 0; i < size; i++) {
        source[i] = d->sectors[sector_number].data[start_offset + offset + i];
    }
}

unsigned int FAT_Device_Sector_read_to_int(FAT_Device *d, FILE *fp, int sector_number,unsigned int start_offset, unsigned int offset, unsigned int size) {
    if (!(d->sectors[sector_number].data)) {
        FAT_Device_load_sector(d, fp, sector_number);
    }
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result += (unsigned char)d->sectors[sector_number].data[start_offset + offset + i] << (i * 8);
    }
    return result;
}

FAT_Dir_Entry FAT_Device_get_dir(FAT_Device *d, FILE *fp, int sector_number, int entry_number)
{
    FAT_Dir_Entry e;
    FAT_Device_load_sector(d, fp, sector_number);

    unsigned int offset = 32 * entry_number;
    FAT_Device_Sector_read_to_char_arr(d, fp, sector_number, e.DIR_Name,offset, 0, 11);
    e.DIR_Attr = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 11, 1);
    e.DIR_NTRes = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 12, 1);
    e.DIR_CrtTimeTenth = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 13, 1);
    e.DIR_CrtTime = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 14, 2);
    e.DIR_CrtDate = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 16, 2);
    e.DIR_LstAccDate = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 18, 2);
    e.DIR_FstClusHI = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 20, 2);
    e.DIR_WrtTime = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 22, 2);
    e.DIR_WrtDate = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 24, 2);
    e.DIR_FstClusLO = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 26, 2);
    e.DIR_FileSize = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 28, 4);

    // print it out
    printf("DIR_Name: ");
    for (int i = 0; i < 11; i++)
    {
        printf("%x ", e.DIR_Name[i]);
//        putchar(e.DIR_Name[i]);
    }
    putchar(10);
    printf("DIR_Attr: %d\n", e.DIR_Attr);
    printf("DIR_NTRes: %d\n", e.DIR_NTRes);
    printf("DIR_CrtTimeTenth: %d\n", e.DIR_CrtTimeTenth);
    printf("DIR_CrtTime: %d\n", e.DIR_CrtTime);
    printf("DIR_CrtDate: %d\n", e.DIR_CrtDate);
    printf("DIR_LstAccDate: %d\n", e.DIR_LstAccDate);
    printf("DIR_FstClusHI: %d\n", e.DIR_FstClusHI);
    printf("DIR_WrtTime: %d\n", e.DIR_WrtTime);
    printf("DIR_WrtDate: %d\n", e.DIR_WrtDate);
    printf("DIR_FstClusLO: %d\n", e.DIR_FstClusLO);
    printf("DIR_FileSize: %d\n", e.DIR_FileSize);

    return e;
}
