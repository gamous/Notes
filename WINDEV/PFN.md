



[Inside Windows Page Frame Number (PFN) - Part 1 | Rayanfam Blog](https://rayanfam.com/topics/inside-windows-page-frame-number-part1/)



Page Frame Number



```
1: kd> dt _MMPFN
nt!_MMPFN
   +0x000 ListEntry        : _LIST_ENTRY
   +0x000 TreeNode         : _RTL_BALANCED_NODE
   +0x000 u1               : <anonymous-tag>
   +0x008 PteAddress       : Ptr64 _MMPTE
   +0x008 PteLong          : Uint8B
   +0x010 OriginalPte      : _MMPTE
   +0x018 u2               : _MIPFNBLINK
   +0x020 u3               : <anonymous-tag>
   +0x024 NodeBlinkLow     : Uint2B
   +0x026 Unused           : Pos 0, 4 Bits
   +0x026 Unused2          : Pos 4, 4 Bits
   +0x027 ViewCount        : UChar
   +0x027 NodeFlinkLow     : UChar
   +0x027 ModifiedListBucketIndex : Pos 0, 4 Bits
   +0x027 AnchorLargePageSize : Pos 0, 2 Bits
   +0x028 u4               : <anonymous-tag>

```



```
1: kd> dd MmPfnDatabase
fffff800`826fc508  00000000 ffff8f00 00000000 00000000
fffff800`826fc518  366ba820 ffffab83 00000006 00000000
fffff800`826fc528  00000001 00000000 00000000 00000000
fffff800`826fc538  ee568000 ffffbb81 366baf00 ffffab83
fffff800`826fc548  00000000 00000000 00000010 00000000
fffff800`826fc558  00000000 0002625a 0002625a 00000000
fffff800`826fc568  366b50c0 ffffab83 00000000 00000002
fffff800`826fc578  00002710 00000000 00000000 00000000

```

