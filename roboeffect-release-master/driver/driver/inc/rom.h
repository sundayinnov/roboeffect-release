/**
 *****************************************************************************
 * @file     rom.h
 * @author   Ben
 * @version  V1.0.0
 * @date     24-04-2018
 * @brief    ROM function application interface for O26
 *
 * @attention:
 * 1.Download MVA package to the bank b start address in flash(e.g. 0x80000)
 * 2.Execute the following code:
 *   ROM_BOOT_DBUpgrade_Apply(1 , 0x80000);//or ROM_BOOT_DBUpgrade_Apply(0,0);
 *   ROM_BOOT_DBUpgrade_Reset();
 * 3.The MCU will auto boot to dual bank upgrade function after reset.Please wait for some time before boot to a new application code.
 * 4.The upgrade executing time is related to the code size and CRC check size.Please refer to the relevant documentation.
 *
 *
 * <h2><center>&copy; COPYRIGHT 2018 MVSilicon </center></h2>
 */

#ifndef DRIVER_INC_ROM_H_
#define DRIVER_INC_ROM_H_

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
/*
 * error code definition in dualbank upgrade progress
 * */
typedef enum _UPGRADE_ERRNO{
	ERRNO_NONE = 0,				//0 没有错误
	ERRNO_NO_MVA_ERR, 			//1 没有找到MVA包
	ERRNO_MVA_CRC_ERR, 			//2 MVA CRC错误
	ERRNO_FLASH_DRV_ERR, 		//3 FLASH DRV 错误
	ERRNO_UNLOCK_ERR, 			//4 FLASH 解锁错误
	ERRNO_LOCK_ERR, 			//5 FLASH 加锁错误
	ERRNO_BANKA_CRC_ERR, 	    //6 BANK A crc校验错误
	ERRNO_FLASH_ERASE_ERR, 		//7 FLASH 擦除错误
	ERRNO_UPGRADED_CRC_ERR, 	//8 升级后CRC错误
	ERRNO_CODE_SIZE_OVERFLOW_ERR, //9 升级包Code size超出
	ERRNO_UNKNOWN_ERR, 			//10 未知错误
	ERRNO_KEY_MISMATCH_ERR,		//11 key 不一致性错误
	ERRNO_UPGRADESIZE_OVERFLOW_MVA_OFF,	//12  升级包的code size 大于MVA包offset
	STATE_NEED_UPGRADE,         //13  需要升级
	STATE_NO_NEED_UPGRADE,      //14不需要升级
	STATE_UPGRADE_SUCCESS       //15  升级成功
}ENUM_UPGRADE_ERRNO;//最多16个状态

/*****************************************************************************************************************
*		User Defined Register Operating functions:
*		    ROM_UserRegisterSet()
*		    ROM_UserRegisterClear()
*		    ROM_UserRegisterGet()
*		This 16 bits Register will lost data if power off,and will hold the data after system reset or MCU reset.
*		用户自定义寄存器（16bit）操作函数 ：
*		重启操作不会丢失  掉电丢失
******************************************************************************************************************/
void ROM_UserRegisterSet(uint16_t setData);

void ROM_UserRegisterClear(void);

uint16_t ROM_UserRegisterGet(void);

/**
 * @brief     Apply for dual bank upgrade
 *
 * @param[in] upgradeMode ∈[0 1]
 *                1: mode 1,apply upgrade by assigning the bank b start address
 *                0: mode 0,apply upgrade by auto searching MAV package header that no need address
 * @param[in] bBankAddr  =0x1000*n n∈(0 0x1000)  start address of bank b who store the MVA package header
 * @return    apply result ∈[-1 0 1]
 *                1  apply mode 1 success.
 *                -1 apply mode 1 failure.   Error address error
 *                0  apply mode 0 success.

 * 功能：用于双bank升级配置升级请求
 * 参数：upgradeMode：     upgradeMode = 1 : 模式1,配置指定bank b地址的升级请求.
 *                     upgradeMode = 0 : 模式0,配置强制升级请求,自动搜寻MVA包首地址.此时不关心bBankAddr.
 *       bBankAddr:    MVA升级包放置首地址，该地址要0x1000=4096对齐 。
 * 返回值：
 *        1   模式1配置成功
 *        -1  模式1配置失败
 *        0   模式0配置成功
 * 备注：配置升级请求后，需要SYS RESET，再进行
 */
int8_t ROM_BankBUpgradeApply(int8_t upgradeMode,uint32_t bBankAddr);

/*
 * @brief   SysReset after Applying for dual bank upgrade
 *
 * 功能：用于双bank升级后进行系统复位
 */
void ROM_BankBUpgradeSysReset(void);

/*
 * @brief   Get return code for dual bank upgrade
 *
 * 功能：获取双bank升级的返回码
 */
ENUM_UPGRADE_ERRNO ROM_BankBUpgradeReturnCodeGet(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif /* DRIVER_INC_ROM_H_ */
