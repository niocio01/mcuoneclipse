/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

/*! @name PORTA1 (number 23), J2[4]/RED_LED
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_LEDRGB_RED_GPIO GPIOA               /*!<@brief GPIO peripheral base pointer */
#define BOARD_LEDRGB_RED_GPIO_PIN_MASK (1U << 1U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_LEDRGB_RED_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_LEDRGB_RED_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_LEDRGB_RED_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                  /* @} */

/*! @name PORTA2 (number 24), J1[8]/GREEN_LED
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_LEDRGB_GREEN_GPIO GPIOA               /*!<@brief GPIO peripheral base pointer */
#define BOARD_LEDRGB_GREEN_GPIO_PIN_MASK (1U << 2U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_LEDRGB_GREEN_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_LEDRGB_GREEN_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_LEDRGB_GREEN_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                    /* @} */

/*! @name PORTD5 (number 62), SPICLK
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_LEDRGB_BLUE_GPIO GPIOD               /*!<@brief GPIO peripheral base pointer */
#define BOARD_LEDRGB_BLUE_GPIO_PIN_MASK (1U << 5U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_LEDRGB_BLUE_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_LEDRGB_BLUE_PIN 5U                   /*!<@brief PORT pin number */
#define BOARD_LEDRGB_BLUE_PIN_MASK (1U << 5U)      /*!<@brief PORT pin mask */
                                                   /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLEDsPins(void);

/*! @name PORTB17 (number 40), MISO
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_SW3_GPIO GPIOB                /*!<@brief GPIO peripheral base pointer */
#define BOARD_SW3_GPIO_PIN_MASK (1U << 17U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_SW3_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_SW3_PIN 17U                   /*!<@brief PORT pin number */
#define BOARD_SW3_PIN_MASK (1U << 17U)      /*!<@brief PORT pin mask */
                                            /* @} */

/*! @name PORTC1 (number 44), J24[6]/LLWU_P6/ADC0_SE15/PUSH_BUTTON2
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_SW2_GPIO GPIOC               /*!<@brief GPIO peripheral base pointer */
#define BOARD_SW2_GPIO_PIN_MASK (1U << 1U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_SW2_PORT PORTC               /*!<@brief PORT peripheral base pointer */
#define BOARD_SW2_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_SW2_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                           /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitButtonsPins(void);

#define SOPT5_UART1TXSRC_UART_TX 0x00u /*!<@brief UART 1 transmit data source select: UART1_TX pin */

/*! @name PORTE1 (number 2), J2[20]/UART1_RX_TGTMCU
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_DEBUG_UART_RX_PORT PORTE               /*!<@brief PORT peripheral base pointer */
#define BOARD_DEBUG_UART_RX_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_RX_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*! @name PORTE0 (number 1), J2[18]/UART1_TX_TGTMCU
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_DEBUG_UART_TX_PORT PORTE               /*!<@brief PORT peripheral base pointer */
#define BOARD_DEBUG_UART_TX_PIN 0U                   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_TX_PIN_MASK (1U << 0U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitDEBUG_UARTPins(void);

/*! @name PORTB2 (number 37), J24[12]/U8[4]/ADC0_SE12/I2C0_SCL/AUD/ACCEL_I2C/POT_5K
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_SCL_PORT PORTB               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_SCL_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_SCL_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                 /* @} */

/*! @name PORTB3 (number 38), J24[10]/U8[6]/ADC0_SE13/I2C0_SDA/AUD/ACCEL_I2C
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_SDA_PORT PORTB               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_SDA_PIN 3U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_SDA_PIN_MASK (1U << 3U)      /*!<@brief PORT pin mask */
                                                 /* @} */

/*! @name PORTD0 (number 57), U8[11]/LLWU_P12/ACCEL_INT1
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_ACCEL_INT1_GPIO GPIOD               /*!<@brief GPIO peripheral base pointer */
#define BOARD_ACCEL_INT1_GPIO_PIN_MASK (1U << 0U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_INT1_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_INT1_PIN 0U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_INT1_PIN_MASK (1U << 0U)      /*!<@brief PORT pin mask */
                                                  /* @} */

/*! @name PORTD1 (number 58), U8[9]/J8[P5]/SPI0_SCK/uSD_SPI_CL/ACCEL_INT2
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_ACCEL_INT2_GPIO GPIOD               /*!<@brief GPIO peripheral base pointer */
#define BOARD_ACCEL_INT2_GPIO_PIN_MASK (1U << 1U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_INT2_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_INT2_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_INT2_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                  /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitAccelPins(void);

/*! @name PORTC4 (number 49), J8[P2]/J24[9]/uSD_card_CS
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_SD_CARD_DAT3_PORT PORTC               /*!<@brief PORT peripheral base pointer */
#define BOARD_SD_CARD_DAT3_PIN 4U                   /*!<@brief PORT pin number */
#define BOARD_SD_CARD_DAT3_PIN_MASK (1U << 4U)      /*!<@brief PORT pin mask */
                                                    /* @} */

/*! @name PORTD1 (number 58), U8[9]/J8[P5]/SPI0_SCK/uSD_SPI_CL/ACCEL_INT2
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_SD_CARD_CLK_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_SD_CARD_CLK_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_SD_CARD_CLK_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                   /* @} */

/*! @name PORTD2 (number 59), J1[2]/J8[P3]/uSD_SPI_MOSI
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_SD_CARD_CMD_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_SD_CARD_CMD_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_SD_CARD_CMD_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                   /* @} */

/*! @name PORTD3 (number 60), J1[4]/J8[P7]/SPI0_SIN/uSD_SPI_MISO
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_SD_CARD_DAT0_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_SD_CARD_DAT0_PIN 3U                   /*!<@brief PORT pin number */
#define BOARD_SD_CARD_DAT0_PIN_MASK (1U << 3U)      /*!<@brief PORT pin mask */
                                                    /* @} */

/*! @name PORTB16 (number 39), MOSI
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_SD_CARD_DETECT_GPIO GPIOB                /*!<@brief GPIO peripheral base pointer */
#define BOARD_SD_CARD_DETECT_GPIO_PIN_MASK (1U << 16U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_SD_CARD_DETECT_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_SD_CARD_DETECT_PIN 16U                   /*!<@brief PORT pin number */
#define BOARD_SD_CARD_DETECT_PIN_MASK (1U << 16U)      /*!<@brief PORT pin mask */
                                                       /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitSDHCPins(void);

/*! @name PORTA18 (number 32), Y1[3]/EXTAL
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_EXTAL0_PORT PORTA                /*!<@brief PORT peripheral base pointer */
#define BOARD_EXTAL0_PIN 18U                   /*!<@brief PORT pin number */
#define BOARD_EXTAL0_PIN_MASK (1U << 18U)      /*!<@brief PORT pin mask */
                                               /* @} */

/*! @name PORTA19 (number 33), Y1[1]/XTAL
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_XTAL0_PORT PORTA                /*!<@brief PORT peripheral base pointer */
#define BOARD_XTAL0_PIN 19U                   /*!<@brief PORT pin number */
#define BOARD_XTAL0_PIN_MASK (1U << 19U)      /*!<@brief PORT pin mask */
                                              /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitOSCPins(void);

/*! @name PORTB2 (number 37), J24[12]/U8[4]/ADC0_SE12/I2C0_SCL/AUD/ACCEL_I2C/POT_5K
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_POT_5K_PORT PORTB               /*!<@brief PORT peripheral base pointer */
#define BOARD_POT_5K_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_POT_5K_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                              /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPOTPins(void);

/*! @name ADC0_DP3 (number 11), J24[5]/ADC0_DP3/ADC0_SE3/LIGHT_SNSR
  @{ */
/* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLSENSEPins(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
