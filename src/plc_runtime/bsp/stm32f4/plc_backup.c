#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <plc_backup.h>
#include <plc_hw.h>

#include <plc_config.h>

#define PLC_BKP_SIZE 4096

#define PLC_UVP(a) ((void *)(a))
//Use RTC backup registers for validation
#define PLC_BKP_VER1   MMIO32(RTC_BKP_BASE + PLC_BKP_VER1_OFFSET)
#define PLC_BKP_VER2   MMIO32(RTC_BKP_BASE + PLC_BKP_VER2_OFFSET)
//Use BackupSRAM to store retain vars
#define PLC_BKP_START PLC_UVP(BKPSRAM_BASE)

//watches RTC_BKP_BASE 0x40002850
//  PLC_BKP_START 0x40024000

void plc_backup_init(void)
{
    rcc_periph_clock_enable( RCC_PWR );
    rcc_periph_clock_enable( RCC_BKPSRAM );

    if( !( PWR_CSR & PWR_CSR_BRE ) )
    {
        pwr_disable_backup_domain_write_protect();

        PWR_CSR |= PWR_CSR_BRE;             // Backup regulator enable
        while( !( PWR_CSR & PWR_CSR_BRR ) );// Wait for backup regulator ready

        pwr_enable_backup_domain_write_protect();
    }
}

void plc_backup_invalidate(void)
{
    pwr_disable_backup_domain_write_protect();
    PLC_BKP_VER1++;
    pwr_enable_backup_domain_write_protect();
}

void plc_backup_validate(void)
{
    pwr_disable_backup_domain_write_protect();
    PLC_BKP_VER2 = PLC_BKP_VER1;
    pwr_enable_backup_domain_write_protect();
}

int plc_backup_check(void)
{
    if( PLC_BKP_VER1 != PLC_BKP_VER2 )
    {
        return 0;
    }
    else
    {
        return 1; //Success, now may remind
    }
}

void plc_backup_remind(unsigned int offset, unsigned int count, void *p)
{
    if(offset + count < PLC_BKP_SIZE)
    {
        plc_hw_status |= PLC_HW_ERR_HSE;
        memcpy( p, PLC_BKP_START + offset, count );
    }
}

void plc_backup_retain(unsigned int offset, unsigned int count, void *p)
{
    if(offset + count < PLC_BKP_SIZE)
    {
        pwr_disable_backup_domain_write_protect();
        memcpy( PLC_BKP_START + offset, p, count );
        pwr_enable_backup_domain_write_protect();
    }
}
