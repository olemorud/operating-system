
/* Encoding scheme:
   X(map,                     opcode,                       ext,                          attrs,                        mnemonic,                     argc,                         a1,                           a2,                           a3,                           desc)
   
   map: MAP_PRIMARY,          MAP_0F 
   opcode: 1-byte primary opcode (numeric literal) 
   ext: -1 if none,           else digit (0..7) 
   attrs bitmask: see * below 
   
   Operand shorthands: r8/16/32/64,                         rm8/16/32/64,                 m8/16/32/64,                  imm8/16/32,                   rel8/32,                      moffs8/16/32/64,              AL/AX/EAX/RAX,                rFLAGS. */

#include <stdint.h>

/* --- enums and flags --- */
enum opcode_map {
    MAP_PRIMARY = 0,
    MAP_0F      = 1,
};

enum arg_kind {
  AK_NONE=0,

  // fixed regs
  AK_AL,
  AK_AX_EAX_RAX,         // accumulator by operand size

  // gp regs
  AK_R8, AK_R16, AK_R32, AK_R64,
  AK_R16_32_64,          // size-selected GPR
  AK_R32_64,             // 32 or 64 reg

  // reg/mem
  AK_RM8, AK_RM16, AK_RM32, AK_RM64,
  AK_RM16_32_64,
  AK_RM8_16_32_64,

  // memory
  AK_M,                  // memory, size from opcode/REX.W
  AK_MOFFS8,
  AK_MOFFS16_32_64,

  // immediates and rels
  AK_IMM8, AK_IMM16, AK_IMM32,
  AK_IMM_OPSIZE,         // 16 or 32 by operand size
  AK_REL8, AK_REL32,

  // special implicit operands
  AK_CL,
  AK_ONE
};

int arg_kind_bytes[] = {
  [AK_NONE] = -9999,
  [AK_AL] = 0,
  [AK_AX_EAX_RAX] = 0,         // accumulator by operand size

  // gp regs
  [AK_R8] = 1,
  [AK_R16] = 2,
  [AK_R32] = 4,
  [AK_R64] = 8,
  [AK_R16_32_64] = -9999,          // size-selected GPR
  [AK_R32_64] = -9999,             // 32 or 64 reg

  // reg/mem
  [AK_RM8] = 1,
  [AK_RM16] = 2,
  [AK_RM32] = 4, 
  [AK_RM64] = 8,
  [AK_RM16_32_64] = -9999,
  [AK_RM8_16_32_64] = -9999,

  // memory
  [AK_M] = -9999,                  // memory, size from opcode/REX.W
  [AK_MOFFS8] = -9999,
  [AK_MOFFS16_32_64] = -9999,

  // immediates and rels
  [AK_IMM8] = 1, 
  [AK_IMM16] = 2,
  [AK_IMM32] = 4,
  [AK_IMM_OPSIZE] = -9999,         // 16 or 32 by operand size
  [AK_REL8] = 1,
  [AK_REL32] = 4,

  // special implicit operands
  [AK_CL] = 0,
  [AK_ONE] = 0
};

const char* arg_kind_str[] = {
    [AK_NONE]          = "NONE",
    [AK_AL]            = "AL",
    [AK_AX_EAX_RAX]    = "AX_EAX_RAX",
    [AK_R8]            = "R8",
    [AK_R16]           = "R16",
    [AK_R32]           = "R32",
    [AK_R64]           = "R64",
    [AK_R16_32_64]     = "R16_32_64",
    [AK_R32_64]        = "R32_64",
    [AK_RM8]           = "RM8",
    [AK_RM16]          = "RM16",
    [AK_RM32]          = "RM32",
    [AK_RM64]          = "RM64",
    [AK_RM16_32_64]    = "RM16_32_64",
    [AK_RM8_16_32_64]  = "RM8_16_32_64",
    [AK_M]             = "M",
    [AK_MOFFS8]        = "MOFFS8",
    [AK_MOFFS16_32_64] = "MOFFS16_32_64",
    [AK_IMM8]          = "IMM8",
    [AK_IMM16]         = "IMM16",
    [AK_IMM32]         = "IMM32",
    [AK_IMM_OPSIZE]    = "IMM_OPSIZE",
    [AK_REL8]          = "REL8",
    [AK_REL32]         = "REL32",
    [AK_CL]            = "CL",
    [AK_ONE]           = "ONE",
};

enum {
  AA_NONE  = 0,
  AA_SEXT  = 1<<0,       // sign-extended when used (e.g., 83 /? imm8)
  AA_ZEXT  = 1<<1        // zero-extended use
};

const char* arg_attr_str[] = {
  [AA_NONE]  = "NONE",
  [AA_SEXT]  = "SEXT",
  [AA_ZEXT]  = "ZEXT",
  [AA_SEXT | AA_ZEXT] = "SEXT + ZEXT",
};

#define ARG(kind, attrs)   ( ((uint32_t)(kind) << 16) | (uint32_t)(attrs) )
#define ARG_KIND(a)        (((a) >> 16) & ((1<<16)-1))
#define ARG_ATTRS(a)       ((a) & ((1<<16)-1))
#define A(kind)            ARG((kind), AA_NONE)
#define A0                 ARG(AK_NONE, AA_NONE)

enum {
    MODRM   = 1<<( 0 + 8),           /* instruction uses ModRM */
    IMM8    = 1<<( 1 + 8),           
    IMM16   = 1<<( 2 + 8),           
    IMM32   = 1<<( 3 + 8),           
    REL8    = 1<<( 4 + 8),           
    REL32   = 1<<( 5 + 8),           
    MOFFS   = 1<<( 6 + 8),           /* moffs form (A0..A3) */
    PLUS_R  = 1<<( 7 + 8),           /* opcode is base + reg */
    ACCUM   = 1<<( 8 + 8),           /* accumulator special encoding */
    LOCK_OK = 1<<( 9 + 8),           /* LOCK prefix allowed */
    REP_F3  = 1<<(10 + 8),           /* REP/REPE F3 meaningful */
    REP_F2  = 1<<(11 + 8),           /* REPNE F2 meaningful */
    REXW_OK = 1<<(12 + 8),           /* 64-bit operand via REX.W */
};

/*
X(opcode, map,                       ext,                         attrs,                        mnemonic,                     argc,                         a1,                            a2,                            a3,                           desc)
*/
#define INSTRUCTION_LIST \
X(0x00, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                ADD,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "Add r8 to r/m8")             \
X(0x01, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        ADD,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Add r to r/m")               \
X(0x02, MAP_PRIMARY,                -1,                           MODRM,                        ADD,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "Add r/m8 to r8")             \
X(0x03, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                ADD,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Add r/m to r")               \
X(0x04, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   ADD,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "Add imm8 to AL")             \
X(0x05, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          ADD,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "Add imm to accumulator")     \
X(0x08, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                OR,                           2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "OR r8 to r/m8")              \
X(0x09, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        OR,                           2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "OR r to r/m")                \
X(0x0A, MAP_PRIMARY,                -1,                           MODRM,                        OR,                           2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "OR r/m8 to r8")              \
X(0x0B, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                OR,                           2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "OR r/m to r")                \
X(0x0C, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   OR,                           2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "OR imm8 to AL")              \
X(0x0D, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          OR,                           2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "OR imm to accumulator")      \
X(0x0F, MAP_PRIMARY,                -1,                           0,                            NOP_MULTI,                    0,                            A0,                            A0,                            A0,                           "Multi-byte NOPs via 0F 1F */r")                            \
X(0x0F, MAP_PRIMARY,                -1,                           0,                            TWO_BYTE_ESC,                 0,                            A0,                            A0,                            A0,                           "Opcode map escape (placeholder)")                          \
X(0x10, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                ADC,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "Add with carry")             \
X(0x11, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        ADC,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Add with carry")             \
X(0x12, MAP_PRIMARY,                -1,                           MODRM,                        ADC,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "Add with carry")             \
X(0x13, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                ADC,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Add with carry")             \
X(0x14, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   ADC,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "ADC imm8 to AL")             \
X(0x15, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          ADC,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "ADC imm to accumulator")     \
X(0x18, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                SBB,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "Subtract with borrow")       \
X(0x19, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        SBB,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Subtract with borrow")       \
X(0x1A, MAP_PRIMARY,                -1,                           MODRM,                        SBB,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "Subtract with borrow")       \
X(0x1B, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                SBB,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Subtract with borrow")       \
X(0x1C, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   SBB,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "SBB imm8 to AL")             \
X(0x1D, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          SBB,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "SBB imm to accumulator")     \
X(0x20, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                AND,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "AND r8 to r/m8")             \
X(0x21, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        AND,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "AND r to r/m")               \
X(0x22, MAP_PRIMARY,                -1,                           MODRM,                        AND,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "AND r/m8 to r8")             \
X(0x23, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                AND,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "AND r/m to r")               \
X(0x24, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   AND,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "AND imm8 to AL")             \
X(0x25, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          AND,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "AND imm to accumulator")     \
X(0x27, MAP_PRIMARY,                -1,                           0,                            DAA,                          0,                            A0,                            A0,                            A0,                           "Decimal adjust after add")   \
X(0x28, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                SUB,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "Subtract r8 from r/m8")      \
X(0x29, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        SUB,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Subtract r from r/m")        \
X(0x2A, MAP_PRIMARY,                -1,                           MODRM,                        SUB,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "Subtract r/m8 from r8")      \
X(0x2B, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                SUB,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Subtract r/m from r")        \
X(0x2C, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   SUB,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "SUB imm8 from AL")           \
X(0x2D, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          SUB,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "SUB imm from accumulator")   \
X(0x2F, MAP_PRIMARY,                -1,                           0,                            DAS,                          0,                            A0,                            A0,                            A0,                           "Decimal adjust after sub")   \
X(0x30, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                XOR,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "XOR r8 to r/m8")             \
X(0x31, MAP_0F,                     -1,                           0,                            RDTSC,                        0,                            A0,                            A0,                            A0,                           "Read time-stamp counter")    \
X(0x31, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        XOR,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "XOR r to r/m")               \
X(0x32, MAP_PRIMARY,                -1,                           MODRM,                        XOR,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "XOR r/m8 to r8")             \
X(0x33, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                XOR,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "XOR r/m to r")               \
X(0x34, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   XOR,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "XOR imm8 to AL")             \
X(0x35, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          XOR,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "XOR imm to accumulator")     \
X(0x37, MAP_PRIMARY,                -1,                           0,                            AAA,                          0,                            A0,                            A0,                            A0,                           "ASCII adjust after add")     \
X(0x38, MAP_PRIMARY,                -1,                           MODRM,                        CMP,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "Compare r/m8, r8")           \
X(0x39, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                CMP,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Compare r/m, r")             \
X(0x3A, MAP_PRIMARY,                -1,                           MODRM,                        CMP,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "Compare r8, r/m8")           \
X(0x3B, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                CMP,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Compare r, r/m")             \
X(0x3C, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   CMP,                          2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "Compare AL, imm8")           \
X(0x3D, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          CMP,                          2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "Compare accumulator, imm")   \
X(0x3F, MAP_PRIMARY,                -1,                           0,                            AAS,                          0,                            A0,                            A0,                            A0,                           "ASCII adjust after sub")     \
X(0x40, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVO,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move O")         \
X(0x41, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVNO,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move NO")        \
X(0x42, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVB,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move B")         \
X(0x43, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVAE,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move AE")        \
X(0x44, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVE,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move E")         \
X(0x45, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVNE,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move NE")        \
X(0x46, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVBE,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move BE")        \
X(0x47, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVA,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move A")         \
X(0x48, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVS,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move S")         \
X(0x49, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVNS,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move NS")        \
X(0x4A, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVP,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move P")         \
X(0x4B, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVNP,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move NP")        \
X(0x4C, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVL,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move L")         \
X(0x4D, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVGE,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move GE")        \
X(0x4E, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVLE,                       2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move LE")        \
X(0x4F, MAP_0F,                     -1,                           MODRM|REXW_OK,                CMOVG,                        2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Conditional move G")         \
X(0x50, MAP_PRIMARY,                -1,                           PLUS_R,                       PUSH,                         1,                            A(AK_R64),                     A0,                            A0,                           "Push r64")                   \
X(0x58, MAP_PRIMARY,                -1,                           PLUS_R,                       POP,                          1,                            A(AK_R64),                     A0,                            A0,                           "Pop into r64")               \
X(0x63, MAP_PRIMARY,                -1,                           MODRM,                        MOVSXD,                       2,                            A(AK_R64),                     A(AK_RM32),                    A0,                           "Sign-extend dword -> qword") \
X(0x68, MAP_PRIMARY,                -1,                           IMM32,                        PUSH,                         1,                            ARG(AK_IMM32, AA_SEXT),        A0,                            A0,                           "Push sign-extended imm32")   \
X(0x70, MAP_PRIMARY,                -1,                           REL8,                         JO,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if overflow")           \
X(0x71, MAP_PRIMARY,                -1,                           REL8,                         JNO,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if not overflow")       \
X(0x72, MAP_PRIMARY,                -1,                           REL8,                         JB,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if below")              \
X(0x73, MAP_PRIMARY,                -1,                           REL8,                         JAE,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if above or equal")     \
X(0x74, MAP_PRIMARY,                -1,                           REL8,                         JE,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if equal")              \
X(0x75, MAP_PRIMARY,                -1,                           REL8,                         JNE,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if not equal")          \
X(0x76, MAP_PRIMARY,                -1,                           REL8,                         JBE,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if below or equal")     \
X(0x77, MAP_PRIMARY,                -1,                           REL8,                         JA,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if above")              \
X(0x78, MAP_PRIMARY,                -1,                           REL8,                         JS,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if sign")               \
X(0x79, MAP_PRIMARY,                -1,                           REL8,                         JNS,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if not sign")           \
X(0x7A, MAP_PRIMARY,                -1,                           REL8,                         JP,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if parity")             \
X(0x7B, MAP_PRIMARY,                -1,                           REL8,                         JNP,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if not parity")         \
X(0x7C, MAP_PRIMARY,                -1,                           REL8,                         JL,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if less")               \
X(0x7D, MAP_PRIMARY,                -1,                           REL8,                         JGE,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if greater or equal")   \
X(0x7E, MAP_PRIMARY,                -1,                           REL8,                         JLE,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if less or equal")      \
X(0x7F, MAP_PRIMARY,                -1,                           REL8,                         JG,                           1,                            A(AK_REL8),                    A0,                            A0,                           "Jump if greater")            \
X(0x80, MAP_0F,                     -1,                           REL32,                        JO,                           1,                            A(AK_REL32),                   A0,                            A0,                           "Near JO")                    \
X(0x80, MAP_PRIMARY,                0,                            MODRM|IMM8|LOCK_OK,           ADD,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Add imm8")                   \
X(0x80, MAP_PRIMARY,                1,                            MODRM|IMM8|LOCK_OK,           OR,                           2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "OR imm8")                    \
X(0x80, MAP_PRIMARY,                2,                            MODRM|IMM8|LOCK_OK,           ADC,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "ADC imm8")                   \
X(0x80, MAP_PRIMARY,                3,                            MODRM|IMM8|LOCK_OK,           SBB,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "SBB imm8")                   \
X(0x80, MAP_PRIMARY,                4,                            MODRM|IMM8|LOCK_OK,           AND,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "AND imm8")                   \
X(0x80, MAP_PRIMARY,                5,                            MODRM|IMM8|LOCK_OK,           SUB,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "SUB imm8")                   \
X(0x80, MAP_PRIMARY,                6,                            MODRM|IMM8|LOCK_OK,           XOR,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "XOR imm8")                   \
X(0x80, MAP_PRIMARY,                7,                            MODRM|IMM8,                   CMP,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Compare r/m8, imm8")         \
X(0x81, MAP_0F,                     -1,                           REL32,                        JNO,                          1,                            A(AK_REL32),                   A0,                            A0,                           "Near JNO")                   \
X(0x81, MAP_PRIMARY,                0,                            MODRM|IMM32|LOCK_OK|REXW_OK,  ADD,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "Add imm")                    \
X(0x81, MAP_PRIMARY,                1,                            MODRM|IMM32|LOCK_OK|REXW_OK,  OR,                           2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "OR imm")                     \
X(0x81, MAP_PRIMARY,                2,                            MODRM|IMM32|LOCK_OK|REXW_OK,  ADC,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "ADC imm")                    \
X(0x81, MAP_PRIMARY,                3,                            MODRM|IMM32|LOCK_OK|REXW_OK,  SBB,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "SBB imm")                    \
X(0x81, MAP_PRIMARY,                4,                            MODRM|IMM32|LOCK_OK|REXW_OK,  AND,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "AND imm")                    \
X(0x81, MAP_PRIMARY,                5,                            MODRM|IMM32|LOCK_OK|REXW_OK,  SUB,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "SUB imm")                    \
X(0x81, MAP_PRIMARY,                6,                            MODRM|IMM32|LOCK_OK|REXW_OK,  XOR,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "XOR imm")                    \
X(0x81, MAP_PRIMARY,                7,                            MODRM|IMM32|REXW_OK,          CMP,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "Compare r/m, imm")           \
X(0x83, MAP_PRIMARY,                0,                            MODRM|IMM8|LOCK_OK|REXW_OK,   ADD,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "Add sign-imm8")              \
X(0x83, MAP_PRIMARY,                1,                            MODRM|IMM8|LOCK_OK|REXW_OK,   OR,                           2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "OR sign-imm8")               \
X(0x83, MAP_PRIMARY,                2,                            MODRM|IMM8|LOCK_OK|REXW_OK,   ADC,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "ADC sign-imm8")              \
X(0x83, MAP_PRIMARY,                3,                            MODRM|IMM8|LOCK_OK|REXW_OK,   SBB,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "SBB sign-imm8")              \
X(0x83, MAP_PRIMARY,                4,                            MODRM|IMM8|LOCK_OK|REXW_OK,   AND,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "AND sign-imm8")              \
X(0x83, MAP_PRIMARY,                5,                            MODRM|IMM8|LOCK_OK|REXW_OK,   SUB,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "SUB sign-imm8")              \
X(0x83, MAP_PRIMARY,                6,                            MODRM|IMM8|LOCK_OK|REXW_OK,   XOR,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "XOR sign-imm8")              \
X(0x83, MAP_PRIMARY,                7,                            MODRM|IMM8|REXW_OK,           CMP,                          2,                            A(AK_RM16_32_64),              ARG(AK_IMM8, AA_SEXT),         A0,                           "Compare r/m, sign-imm8")     \
X(0x84, MAP_PRIMARY,                -1,                           MODRM,                        TEST,                         2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "TEST r/m8, r8")              \
X(0x85, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                TEST,                         2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "TEST r/m, r")                \
X(0x87, MAP_PRIMARY,                -1,                           MODRM|REXW_OK|LOCK_OK,        XCHG,                         2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Exchange")                   \
X(0x88, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK,                MOV,                          2,                            A(AK_RM8),                     A(AK_R8),                      A0,                           "Move r8 -> r/m8")            \
X(0x89, MAP_PRIMARY,                -1,                           MODRM|LOCK_OK|REXW_OK,        MOV,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Move r -> r/m")              \
X(0x8A, MAP_PRIMARY,                -1,                           MODRM,                        MOV,                          2,                            A(AK_R8),                      A(AK_RM8),                     A0,                           "Move r/m8 -> r8")            \
X(0x8B, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                MOV,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Move r/m -> r")              \
X(0x8D, MAP_PRIMARY,                -1,                           MODRM|REXW_OK,                LEA,                          2,                            A(AK_R16_32_64),               A(AK_M),                       A0,                           "Load effective address")     \
X(0x8F, MAP_PRIMARY,                -1,                           0,                            NOP_ALIAS,                    0,                            A0,                            A0,                            A0,                           "Used as POP r/m64; no pure LEA alias here") \
X(0x8F, MAP_PRIMARY,                0,                            MODRM,                        POP,                          1,                            A(AK_RM64),                    A0,                            A0,                           "Pop into r/m64")             \
X(0x90, MAP_0F,                     -1,                           MODRM,                        SETO,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on overflow")            \
X(0x90, MAP_PRIMARY,                -1,                           0,                            NOP,                          0,                            A0,                            A0,                            A0,                           "No operation")               \
X(0x90, MAP_PRIMARY,                -1,                           0,                            PAUSE,                        0,                            A0,                            A0,                            A0,                           "PAUSE uses F3 90 prefix")    \
X(0x90, MAP_PRIMARY,                -1,                           PLUS_R|ACCUM,                 XCHG,                         2,                            A(AK_AX_EAX_RAX),              A(AK_R16_32_64),               A0,                           "Exchange with accumulator")  \
X(0x91, MAP_0F,                     -1,                           MODRM,                        SETNO,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on not overflow")        \
X(0x92, MAP_0F,                     -1,                           MODRM,                        SETB,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on below/CF=1")          \
X(0x93, MAP_0F,                     -1,                           MODRM,                        SETAE,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on above or equal")      \
X(0x94, MAP_0F,                     -1,                           MODRM,                        SETE,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on equal")               \
X(0x95, MAP_0F,                     -1,                           MODRM,                        SETNE,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on not equal")           \
X(0x96, MAP_0F,                     -1,                           MODRM,                        SETBE,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on below or equal")      \
X(0x97, MAP_0F,                     -1,                           MODRM,                        SETA,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on above")               \
X(0x98, MAP_0F,                     -1,                           MODRM,                        SETS,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on sign")                \
X(0x98, MAP_PRIMARY,                -1,                           0,                            CBW,                          0,                            A0,                            A0,                            A0,                           "AL->AX (size-propagates)")   \
X(0x99, MAP_0F,                     -1,                           MODRM,                        SETNS,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on not sign")            \
X(0x99, MAP_PRIMARY,                -1,                           0,                            CWD,                          0,                            A0,                            A0,                            A0,                           "AX->DX:AX (CDQ/CQO by size)")                              \
X(0x9A, MAP_0F,                     -1,                           MODRM,                        SETP,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on parity")              \
X(0x9A, MAP_PRIMARY,                -1,                           0,                            FARCALL,                      0,                            A0,                            A0,                            A0,                           "Far call (not common in 64-bit)")                          \
X(0x9B, MAP_0F,                     -1,                           MODRM,                        SETNP,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on not parity")          \
X(0x9B, MAP_PRIMARY,                -1,                           0,                            FWAIT,                        0,                            A0,                            A0,                            A0,                           "Wait (legacy alias WAIT)")   \
X(0x9C, MAP_0F,                     -1,                           MODRM,                        SETL,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on less")                \
X(0x9C, MAP_PRIMARY,                -1,                           0,                            PUSHF,                        0,                            A0,                            A0,                            A0,                           "Push rFLAGS (alias)")        \
X(0x9C, MAP_PRIMARY,                -1,                           0,                            PUSHFQ,                       0,                            A0,                            A0,                            A0,                           "Push RFLAGS")                \
X(0x9D, MAP_0F,                     -1,                           MODRM,                        SETGE,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on greater or equal")    \
X(0x9D, MAP_PRIMARY,                -1,                           0,                            POPF,                         0,                            A0,                            A0,                            A0,                           "Pop rFLAGS (alias)")         \
X(0x9D, MAP_PRIMARY,                -1,                           0,                            POPFQ,                        0,                            A0,                            A0,                            A0,                           "Pop RFLAGS")                 \
X(0x9E, MAP_0F,                     -1,                           MODRM,                        SETLE,                        1,                            A(AK_RM8),                     A0,                            A0,                           "Set on less or equal")       \
X(0x9E, MAP_PRIMARY,                -1,                           0,                            SAHF,                         0,                            A0,                            A0,                            A0,                           "Store AH into RFLAGS")       \
X(0x9F, MAP_0F,                     -1,                           MODRM,                        SETG,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Set on greater")             \
X(0x9F, MAP_PRIMARY,                -1,                           0,                            LAHF,                         0,                            A0,                            A0,                            A0,                           "Load AH with RFLAGS")        \
X(0xA0, MAP_PRIMARY,                -1,                           MOFFS|ACCUM,                  MOV,                          2,                            A(AK_AL),                      A(AK_MOFFS8),                  A0,                           "Load moffs8 -> AL")          \
X(0xA1, MAP_PRIMARY,                -1,                           MOFFS|ACCUM|REXW_OK,          MOV,                          2,                            A(AK_AX_EAX_RAX),              A(AK_MOFFS16_32_64),           A0,                           "Load moffs -> RAX")          \
X(0xA2, MAP_0F,                     -1,                           0,                            CPUID,                        0,                            A0,                            A0,                            A0,                           "Query CPU ID")               \
X(0xA2, MAP_PRIMARY,                -1,                           MOFFS|ACCUM,                  MOV,                          2,                            A(AK_MOFFS8),                  A(AK_AL),                      A0,                           "Store AL -> moffs8")         \
X(0xA3, MAP_0F,                     -1,                           MODRM|REXW_OK,                BT,                           2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Bit test")                   \
X(0xA3, MAP_PRIMARY,                -1,                           MOFFS|ACCUM|REXW_OK,          MOV,                          2,                            A(AK_MOFFS16_32_64),           A(AK_AX_EAX_RAX),              A0,                           "Store RAX -> moffs")         \
X(0xA4, MAP_PRIMARY,                -1,                           REP_F3|REP_F2,                MOVSB,                        0,                            A0,                            A0,                            A0,                           "Move byte string")           \
X(0xA5, MAP_PRIMARY,                -1,                           REP_F3|REP_F2|REXW_OK,        MOVSWD,                       0,                            A0,                            A0,                            A0,                           "Move string (word/dword/qword by size)")                   \
X(0xA6, MAP_PRIMARY,                -1,                           REP_F3|REP_F2,                CMPSB,                        0,                            A0,                            A0,                            A0,                           "Compare byte string")        \
X(0xA7, MAP_PRIMARY,                -1,                           REP_F3|REP_F2|REXW_OK,        CMPSWD,                       0,                            A0,                            A0,                            A0,                           "Compare string")             \
X(0xA8, MAP_PRIMARY,                -1,                           IMM8|ACCUM,                   TEST,                         2,                            A(AK_AL),                      A(AK_IMM8),                    A0,                           "TEST AL, imm8")              \
X(0xA9, MAP_PRIMARY,                -1,                           IMM32|ACCUM|REXW_OK,          TEST,                         2,                            A(AK_AX_EAX_RAX),              A(AK_IMM_OPSIZE),              A0,                           "TEST accumulator, imm")      \
X(0xAA, MAP_PRIMARY,                -1,                           REP_F3|REP_F2,                STOSB,                        0,                            A0,                            A0,                            A0,                           "Store byte string")          \
X(0xAB, MAP_0F,                     -1,                           MODRM|LOCK_OK|REXW_OK,        BTS,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Bit test and set")           \
X(0xAB, MAP_PRIMARY,                -1,                           REP_F3|REP_F2|REXW_OK,        STOSWD,                       0,                            A0,                            A0,                            A0,                           "Store string")               \
X(0xAC, MAP_PRIMARY,                -1,                           REP_F3|REP_F2,                LODSB,                        0,                            A0,                            A0,                            A0,                           "Load byte string")           \
X(0xAD, MAP_PRIMARY,                -1,                           REP_F3|REP_F2|REXW_OK,        LODSWD,                       0,                            A0,                            A0,                            A0,                           "Load string")                \
X(0xAE, MAP_PRIMARY,                -1,                           REP_F3|REP_F2,                SCASB,                        0,                            A0,                            A0,                            A0,                           "Scan byte string")           \
X(0xAF, MAP_PRIMARY,                -1,                           REP_F3|REP_F2|REXW_OK,        SCASWD,                       0,                            A0,                            A0,                            A0,                           "Scan string")                \
X(0xB3, MAP_0F,                     -1,                           MODRM|LOCK_OK|REXW_OK,        BTR,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Bit test and reset")         \
X(0xB6, MAP_0F,                     -1,                           MODRM|REXW_OK,                MOVZX,                        2,                            A(AK_R16_32_64),               A(AK_RM8),                     A0,                           "Zero-extend byte")           \
X(0xB7, MAP_0F,                     -1,                           MODRM|REXW_OK,                MOVZX,                        2,                            A(AK_R16_32_64),               A(AK_RM16),                    A0,                           "Zero-extend word")           \
X(0xBA, MAP_0F,                     4,                            MODRM|IMM8|REXW_OK,           BT,                           2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Bit test imm")               \
X(0xBA, MAP_0F,                     5,                            MODRM|IMM8|LOCK_OK|REXW_OK,   BTS,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Bit test and set imm")       \
X(0xBA, MAP_0F,                     6,                            MODRM|IMM8|LOCK_OK|REXW_OK,   BTR,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Bit test and reset imm")     \
X(0xBA, MAP_0F,                     7,                            MODRM|IMM8|LOCK_OK|REXW_OK,   BTC,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Bit test and complement imm")\
X(0xBB, MAP_0F,                     -1,                           MODRM|LOCK_OK|REXW_OK,        BTC,                          2,                            A(AK_RM16_32_64),              A(AK_R16_32_64),               A0,                           "Bit test and complement")    \
X(0xBC, MAP_0F,                     -1,                           MODRM|REXW_OK,                BSF,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Bit scan forward")           \
X(0xBD, MAP_0F,                     -1,                           MODRM|REXW_OK,                BSR,                          2,                            A(AK_R16_32_64),               A(AK_RM16_32_64),              A0,                           "Bit scan reverse")           \
X(0xBE, MAP_0F,                     -1,                           MODRM|REXW_OK,                MOVSX,                        2,                            A(AK_R16_32_64),               A(AK_RM8),                     A0,                           "Sign-extend byte")           \
X(0xBF, MAP_0F,                     -1,                           MODRM|REXW_OK,                MOVSX,                        2,                            A(AK_R16_32_64),               A(AK_RM16),                    A0,                           "Sign-extend word")           \
X(0xC0, MAP_0F,                     -1,                           MODRM|REXW_OK|LOCK_OK,        XADD,                         2,                            A(AK_RM8_16_32_64),            A(AK_R16_32_64),               A0,                           "Exchange and add")           \
X(0xC0, MAP_PRIMARY,                0,                            MODRM|IMM8,                   ROL,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Rotate left by imm8")        \
X(0xC0, MAP_PRIMARY,                1,                            MODRM|IMM8,                   ROR,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Rotate right by imm8")       \
X(0xC0, MAP_PRIMARY,                4,                            MODRM|IMM8,                   SHL,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Shift left by imm8")         \
X(0xC0, MAP_PRIMARY,                5,                            MODRM|IMM8,                   SHR,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Logical shift right by imm8")\
X(0xC0, MAP_PRIMARY,                7,                            MODRM|IMM8,                   SAR,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Arithmetic shift right by imm8")\
X(0xC1, MAP_PRIMARY,                0,                            MODRM|IMM8|REXW_OK,           ROL,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Rotate left by imm8")        \
X(0xC1, MAP_PRIMARY,                1,                            MODRM|IMM8|REXW_OK,           ROR,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Rotate right by imm8")       \
X(0xC1, MAP_PRIMARY,                4,                            MODRM|IMM8|REXW_OK,           SHL,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Shift left by imm8")         \
X(0xC1, MAP_PRIMARY,                5,                            MODRM|IMM8|REXW_OK,           SHR,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Logical shift right by imm8")\
X(0xC1, MAP_PRIMARY,                7,                            MODRM|IMM8|REXW_OK,           SAR,                          2,                            A(AK_RM16_32_64),              A(AK_IMM8),                    A0,                           "Arithmetic shift right by imm8")\
X(0xC2, MAP_PRIMARY,                -1,                           IMM16,                        RET,                          1,                            A(AK_IMM16),                   A0,                            A0,                           "Return and pop imm16")       \
X(0xC3, MAP_PRIMARY,                -1,                           0,                            RET,                          0,                            A0,                            A0,                            A0,                           "Return")                     \
X(0xC6, MAP_PRIMARY,                0,                            MODRM|IMM8|LOCK_OK,           MOV,                          2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "Move imm8 -> r/m8")          \
X(0xC7, MAP_PRIMARY,                0,                            MODRM|IMM32|LOCK_OK|REXW_OK,  MOV,                          2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "Move imm -> r/m")            \
X(0xC8, MAP_0F,                     -1,                           PLUS_R|REXW_OK,               BSWAP,                        1,                            A(AK_R32_64),                  A0,                            A0,                           "Byte swap r32/r64")          \
X(0xD0, MAP_PRIMARY,                0,                            MODRM,                        ROL,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Rotate left by 1")           \
X(0xD0, MAP_PRIMARY,                1,                            MODRM,                        ROR,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Rotate right by 1")          \
X(0xD0, MAP_PRIMARY,                2,                            MODRM,                        RCL,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Rotate through CF left by 1")\
X(0xD0, MAP_PRIMARY,                3,                            MODRM,                        RCR,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Rotate through CF right by 1")\
X(0xD0, MAP_PRIMARY,                4,                            MODRM,                        SHL,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Shift left by 1")            \
X(0xD0, MAP_PRIMARY,                5,                            MODRM,                        SHR,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Logical shift right by 1")   \
X(0xD0, MAP_PRIMARY,                7,                            MODRM,                        SAR,                          2,                            A(AK_RM8),                     A(AK_ONE),                     A0,                           "Arithmetic shift right by 1")\
X(0xD1, MAP_PRIMARY,                0,                            MODRM|REXW_OK,                ROL,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Rotate left by 1")           \
X(0xD1, MAP_PRIMARY,                1,                            MODRM|REXW_OK,                ROR,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Rotate right by 1")          \
X(0xD1, MAP_PRIMARY,                2,                            MODRM|REXW_OK,                RCL,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Rotate through CF left by 1")\
X(0xD1, MAP_PRIMARY,                3,                            MODRM|REXW_OK,                RCR,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Rotate through CF right by 1")\
X(0xD1, MAP_PRIMARY,                4,                            MODRM|REXW_OK,                SHL,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Shift left by 1")            \
X(0xD1, MAP_PRIMARY,                5,                            MODRM|REXW_OK,                SHR,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Logical shift right by 1")   \
X(0xD1, MAP_PRIMARY,                7,                            MODRM|REXW_OK,                SAR,                          2,                            A(AK_RM16_32_64),              A(AK_ONE),                     A0,                           "Arithmetic shift right by 1")\
X(0xD2, MAP_PRIMARY,                0,                            MODRM,                        ROL,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Rotate left by CL")          \
X(0xD2, MAP_PRIMARY,                1,                            MODRM,                        ROR,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Rotate right by CL")         \
X(0xD2, MAP_PRIMARY,                2,                            MODRM,                        RCL,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Rotate through CF left by CL")\
X(0xD2, MAP_PRIMARY,                3,                            MODRM,                        RCR,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Rotate through CF right by CL")\
X(0xD2, MAP_PRIMARY,                4,                            MODRM,                        SHL,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Shift left by CL")           \
X(0xD2, MAP_PRIMARY,                5,                            MODRM,                        SHR,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Logical shift right by CL")  \
X(0xD2, MAP_PRIMARY,                7,                            MODRM,                        SAR,                          2,                            A(AK_RM8),                     A(AK_CL),                      A0,                           "Arithmetic shift right by CL")\
X(0xD3, MAP_PRIMARY,                0,                            MODRM|REXW_OK,                ROL,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Rotate left by CL")          \
X(0xD3, MAP_PRIMARY,                1,                            MODRM|REXW_OK,                ROR,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Rotate right by CL")         \
X(0xD3, MAP_PRIMARY,                2,                            MODRM|REXW_OK,                RCL,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Rotate through CF left by CL")\
X(0xD3, MAP_PRIMARY,                3,                            MODRM|REXW_OK,                RCR,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Rotate through CF right by CL")\
X(0xD3, MAP_PRIMARY,                4,                            MODRM|REXW_OK,                SHL,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Shift left by CL")           \
X(0xD3, MAP_PRIMARY,                5,                            MODRM|REXW_OK,                SHR,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Logical shift right by CL")  \
X(0xD3, MAP_PRIMARY,                7,                            MODRM|REXW_OK,                SAR,                          2,                            A(AK_RM16_32_64),              A(AK_CL),                      A0,                           "Arithmetic shift right by CL")\
X(0xD7, MAP_PRIMARY,                -1,                           0,                            XLAT,                         0,                            A0,                            A0,                            A0,                           "Table look-up translation")  \
X(0xE8, MAP_PRIMARY,                -1,                           REL32,                        CALL,                         1,                            A(AK_REL32),                   A0,                            A0,                           "Near call rel32")            \
X(0xE9, MAP_PRIMARY,                -1,                           REL32,                        JMP,                          1,                            A(AK_REL32),                   A0,                            A0,                           "Near jump rel32")            \
X(0xEA, MAP_PRIMARY,                -1,                           0,                            FARJMP,                       0,                            A0,                            A0,                            A0,                           "Far jump (not common in 64-bit)")                          \
X(0xEB, MAP_PRIMARY,                -1,                           REL8,                         JMP,                          1,                            A(AK_REL8),                    A0,                            A0,                           "Short jump rel8")            \
X(0xF4, MAP_PRIMARY,                -1,                           0,                            HLT,                          0,                            A0,                            A0,                            A0,                           "Halt")                       \
X(0xF5, MAP_PRIMARY,                -1,                           0,                            CMC,                          0,                            A0,                            A0,                            A0,                           "Complement CF")              \
X(0xF6, MAP_PRIMARY,                0,                            MODRM|IMM8,                   TEST,                         2,                            A(AK_RM8),                     A(AK_IMM8),                    A0,                           "TEST r/m8, imm8")            \
X(0xF6, MAP_PRIMARY,                2,                            MODRM,                        NOT,                          1,                            A(AK_RM8),                     A0,                            A0,                           "Invert r/m8")                \
X(0xF6, MAP_PRIMARY,                3,                            MODRM,                        NEG,                          1,                            A(AK_RM8),                     A0,                            A0,                           "Two's-complement r/m8")      \
X(0xF6, MAP_PRIMARY,                4,                            MODRM,                        MUL,                          1,                            A(AK_RM8),                     A0,                            A0,                           "Unsigned mul AL * r/m8 -> AX")                             \
X(0xF6, MAP_PRIMARY,                5,                            MODRM,                        IMUL,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Signed mul AL * r/m8 -> AX") \
X(0xF6, MAP_PRIMARY,                6,                            MODRM,                        DIV,                          1,                            A(AK_RM8),                     A0,                            A0,                           "Unsigned div AX / r/m8")     \
X(0xF6, MAP_PRIMARY,                7,                            MODRM,                        IDIV,                         1,                            A(AK_RM8),                     A0,                            A0,                           "Signed div AX / r/m8")       \
X(0xF7, MAP_PRIMARY,                0,                            MODRM|IMM32|REXW_OK,          TEST,                         2,                            A(AK_RM16_32_64),              A(AK_IMM_OPSIZE),              A0,                           "TEST r/m, imm")              \
X(0xF7, MAP_PRIMARY,                2,                            MODRM|REXW_OK,                NOT,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Invert r/m")                 \
X(0xF7, MAP_PRIMARY,                3,                            MODRM|REXW_OK,                NEG,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Two's-complement r/m")       \
X(0xF7, MAP_PRIMARY,                4,                            MODRM|REXW_OK,                MUL,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Unsigned mul accumulator")   \
X(0xF7, MAP_PRIMARY,                5,                            MODRM|REXW_OK,                IMUL,                         1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Signed mul accumulator")     \
X(0xF7, MAP_PRIMARY,                6,                            MODRM|REXW_OK,                DIV,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Unsigned division")          \
X(0xF7, MAP_PRIMARY,                7,                            MODRM|REXW_OK,                IDIV,                         1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Signed division")            \
X(0xF8, MAP_PRIMARY,                -1,                           0,                            CLC,                          0,                            A0,                            A0,                            A0,                           "Clear CF")                   \
X(0xF9, MAP_PRIMARY,                -1,                           0,                            STC,                          0,                            A0,                            A0,                            A0,                           "Set CF")                     \
X(0xFA, MAP_PRIMARY,                -1,                           0,                            CLI,                          0,                            A0,                            A0,                            A0,                           "Clear IF")                   \
X(0xFB, MAP_PRIMARY,                -1,                           0,                            STI,                          0,                            A0,                            A0,                            A0,                           "Set IF")                     \
X(0xFC, MAP_PRIMARY,                -1,                           0,                            CLD,                          0,                            A0,                            A0,                            A0,                           "Clear DF")                   \
X(0xFD, MAP_PRIMARY,                -1,                           0,                            STD,                          0,                            A0,                            A0,                            A0,                           "Set DF")                     \
X(0xFE, MAP_PRIMARY,                0,                            MODRM|LOCK_OK,                INC,                          1,                            A(AK_RM8),                     A0,                            A0,                           "Increment r/m8")             \
X(0xFE, MAP_PRIMARY,                1,                            MODRM|LOCK_OK,                DEC,                          1,                            A(AK_RM8),                     A0,                            A0,                           "Decrement r/m8")             \
X(0xFF, MAP_PRIMARY,                0,                            MODRM|LOCK_OK|REXW_OK,        INC,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Increment r/m")              \
X(0xFF, MAP_PRIMARY,                1,                            MODRM|LOCK_OK|REXW_OK,        DEC,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Decrement r/m")              \
X(0xFF, MAP_PRIMARY,                2,                            MODRM|REXW_OK,                CALL,                         1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Indirect call")              \
X(0xFF, MAP_PRIMARY,                4,                            MODRM|REXW_OK,                JMP,                          1,                            A(AK_RM16_32_64),              A0,                            A0,                           "Indirect jump")              \
X(0xFF, MAP_PRIMARY,                6,                            MODRM,                        PUSH,                         1,                            A(AK_RM64),                    A0,                            A0,                           "Push r/m64")                 \

struct instruction {
    uint8_t     opcode;
    int32_t     ext;
    uint32_t    attrs;
    uint32_t    argc;
    const char* mnemonic;
    uint32_t    args[3];
    const char* desc;
};

#define XSTR(x) #x
#define STR(x) XSTR(x)

static const struct instruction byte_to_instruction[] = {
#define X(_opcode, _map, _ext, _attrs, _mnemonic, _argc, _a1, _a2, _a3, _desc) \
    [_opcode] = {.opcode = _opcode, .ext = _ext, .attrs = _attrs, .mnemonic = STR(_mnemonic), .argc = _argc,  .args = {_a1, _a2, _a3}, .desc = _desc},
    INSTRUCTION_LIST
#undef X
};

#include <stdio.h>

int main()
{
    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if (c < 0 || c > sizeof byte_to_instruction / sizeof *byte_to_instruction) {
            fprintf(stderr, "unsupported byte 0x%02x, aborting\n", c);
            break;
        }
        struct instruction ins = byte_to_instruction[c];
        if (ins.opcode == 0) {
            fprintf(stderr, "unsupported byte 0x%02x, aborting\n", c);
            break;
        }

        printf("OP: %s (0x%02x)\n", ins.mnemonic, ins.opcode);

        for (int i = 0; i < ins.argc; i++) {
            uint32_t arg = ins.args[i];
            enum arg_kind kind = ARG_KIND(arg);
            int attr = ARG_ATTRS(arg);

            union {
                unsigned char bytes[4];
                int32_t i32;
                int32_t u32;
            } val = {0};

            //for (int i = 3; i > 0; i--) {
            for (int i = 0; i < arg_kind_bytes[kind]; i++) {
                c = fgetc(stdin);
                if (c == EOF) goto end;
                val.bytes[i] = c;
            }

            switch (kind) {
            default:
            {
                printf("ARG: kind: %s, attrs: %s, val: 0x%02x\n", arg_kind_str[kind], arg_attr_str[attr], val.u32);
            } break;
            }
        }

    }

end:
    return 0;
}
