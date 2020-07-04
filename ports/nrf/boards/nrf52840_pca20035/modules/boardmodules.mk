BOARD_NRF52840_PCA20035_DIR = boards/nrf52840_pca20035/modules

INC += -I./$(BOARD_NRF52840_PCA20035_DIR)
CFLAGS += -DBOARD_SPECIFIC_MODULES

SRC_BOARD_MODULES = $(addprefix $(BOARD_NRF52840_PCA20035_DIR)/,\
                      recover_uicr_regout0.c \
                      )

OBJ += $(addprefix $(BUILD)/, $(SRC_BOARD_MODULES:.c=.o))

