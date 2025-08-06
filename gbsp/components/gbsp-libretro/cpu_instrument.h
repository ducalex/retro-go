
// Instrumentation and debugging code
// Used to count instruction and register usage
// Also provides some tracing capabilities


#ifdef MEMORY_STATS_ANALYZE
  // Collects memory stats by region, access type, etc.

  u32 memory_region_access_read_u8[16];
  u32 memory_region_access_read_s8[16];
  u32 memory_region_access_read_u16[16];
  u32 memory_region_access_read_s16[16];
  u32 memory_region_access_read_u32[16];
  u32 memory_region_access_write_u8[16];
  u32 memory_region_access_write_u16[16];
  u32 memory_region_access_write_u32[16];

  #define STATS_MEMORY_ACCESS(op, size, region) \
    memory_region_access_##op##_##size[region]++;

#else

  #define STATS_MEMORY_ACCESS(write, u32, region)

#endif

#ifdef REGISTER_USAGE_ANALYZE

  u64 instructions_total = 0;
  u64 arm_reg_freq[16];
  u64 arm_reg_access_total = 0;
  u64 arm_instructions_total = 0;
  u64 thumb_reg_freq[16];
  u64 thumb_reg_access_total = 0;
  u64 thumb_instructions_total = 0;

  // mla/long mla's addition operand are not counted yet.

  #define using_register(instruction_set, register, type)                     \
    instruction_set##_reg_freq[register]++;                                   \
    instruction_set##_reg_access_total++                                      \

  #define using_register_list(instruction_set, rlist, count)                  \
  {                                                                           \
    u32 i;                                                                    \
    for(i = 0; i < count; i++)                                                \
    {                                                                         \
      if((reg_list >> i) & 0x01)                                              \
      {                                                                       \
        using_register(instruction_set, i, memory_target);                    \
      }                                                                       \
    }                                                                         \
  }                                                                           \

  #define using_instruction(instruction_set)                                  \
    instruction_set##_instructions_total++;                                   \
    instructions_total++                                                      \

  int sort_tagged_element(const void *_a, const void *_b)
  {
    const u64 *a = _a;
    const u64 *b = _b;

    return (int)(b[1] - a[1]);
  }

  void print_register_usage(void)
  {
    u32 i;
    u64 arm_reg_freq_tagged[32];
    u64 thumb_reg_freq_tagged[32];
    double percent;
    double percent_total = 0.0;

    for(i = 0; i < 16; i++)
    {
      arm_reg_freq_tagged[i * 2] = i;
      arm_reg_freq_tagged[(i * 2) + 1] = arm_reg_freq[i];
      thumb_reg_freq_tagged[i * 2] = i;
      thumb_reg_freq_tagged[(i * 2) + 1] = thumb_reg_freq[i];
    }

    qsort(arm_reg_freq_tagged, 16, sizeof(u64) * 2, sort_tagged_element);
    qsort(thumb_reg_freq_tagged, 16, sizeof(u64) * 2, sort_tagged_element);

    printf("ARM register usage (%lf%% ARM instructions):\n",
     (arm_instructions_total * 100.0) / instructions_total);
    for(i = 0; i < 16; i++)
    {
      percent = (arm_reg_freq_tagged[(i * 2) + 1] * 100.0) /
       arm_reg_access_total;
      percent_total += percent;
      printf("r%02d: %lf%% (-- %lf%%)\n",
       (u32)arm_reg_freq_tagged[(i * 2)], percent, percent_total);
    }

    percent_total = 0.0;

    printf("\nThumb register usage (%lf%% Thumb instructions):\n",
     (thumb_instructions_total * 100.0) / instructions_total);
    for(i = 0; i < 16; i++)
    {
      percent = (thumb_reg_freq_tagged[(i * 2) + 1] * 100.0) /
       thumb_reg_access_total;
      percent_total += percent;
      printf("r%02d: %lf%% (-- %lf%%)\n",
       (u32)thumb_reg_freq_tagged[(i * 2)], percent, percent_total);
    }

    memset(arm_reg_freq, 0, sizeof(u64) * 16);
    memset(thumb_reg_freq, 0, sizeof(u64) * 16);
    arm_reg_access_total = 0;
    thumb_reg_access_total = 0;
  }

#else  /* REGISTER_USAGE_ANALYZE */

  #define using_register(instruction_set, register, type)
  #define using_register_list(instruction_set, rlist, count)
  #define using_instruction(instruction_set)

#endif  /* REGISTER_USAGE_ANALYZE */


#ifdef TRACE_INSTRUCTIONS
  static void interp_trace_instruction(u32 pc, u32 mode)
  {
    if (mode)
      printf("Executed arm %x\n", pc);
    else
      printf("Executed thumb %x\n", pc);
    #ifdef TRACE_REGISTERS
    print_regs();
    #endif
  }
#endif

