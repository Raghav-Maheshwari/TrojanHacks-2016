/* definitions for LAURA */

// i2c slave address
#define addrLAURA 0x4C

/**
 * Registers Addresses
 * -> Base Addr: 0x320            Address
 */
#define addr_ADDA_ICSR            0x00 // 0x0300
#define addr_ADDA_IER             0x02 // 0x0302
#define addr_ADDA_CMD             0x04 // 0x0304
#define addr_ADDA_DSTATUS         0x06 // 0x0306
#define addr_ADDA_RSLTR           0x08 // 0x0308
#define addr_ADDA_RSL_CON         0x0A // 0x030A
#define addr_ADDA_RFSH            0x0C // 0x030C
#define addr_ADDA_AUXCTRL         0x0E // 0x030E
#define addr_ADDA_PMU_CFG_UP      0x15 // 0x0315
#define addr_I2C_ACC_CFG          0x1C // 0x031C
#define addr_ADDA_HFCFG0          0x20 // 0x0320
#define addr_ADDA_HFCFG1          0x22 // 0x0322
#define addr_ADDA_HFCFG2          0x24 // 0x0324
#define addr_ADDA_HFCFG3          0x26 // 0x0326
#define addr_ADDA_CHIP_ID         0x28 // 0x0328

/**
 * Interrupt Control and Status Register (1 byte)
 */
#define ICSR_INT0_CLR_RES_N_ICSR  (1 << 0)
#define ICSR_INT_RESULT           (1 << 4)
#define ICSR_INT_M2H_REQ          (1 << 5)
#define ICSR_INT_M2H_ACK          (1 << 6)
#define ICSR_INT_RESET            (1 << 7)

/**
 * Interrupt Enable Register (1 byte)
 */
#define IER_ENABLE_MASK           ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3))
#define IER_CLEAR_MASK            ((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7))

/**
 * Command Register (1 byte)
 */
#define CMD_SINGLE_MEASURE        0x01
#define CMD_START_MEASURE         0x02
#define CMD_GO_STANDBY            0x10
#define CMD_VALID_MASK            (1 << 7)

/**
 * Device Status Register (2 bytes)
 */
#define DSTATUS_PM_STATUS_MASK    0x001F
#define DSTATUS_PM_STATUS_MCPU_ON 0x18

/**
 * I2C Access Configuration Register (1 byte)
 */
#define I2C_AUTO_INC              (1 << 0)
#define I2C_AUTO_INC_DATAP        (1 << 2)
#define I2C_MCPU_DEBUG_ACC        (1 << 6)
