class GSL1680 {

    public:
        GSL1680();
        GSL1680(bool error, bool info);
        void begin(uint8_t WAKE, uint8_t INTRPT);       
        struct Touch_event {
            uint16_t x1;
            uint16_t y1;
            uint16_t x2;
            uint16_t y2;
            uint16_t x3;
            uint16_t y3;
            uint16_t x4;
            uint16_t y4;
            uint16_t x5;
            uint16_t y5;
            uint8_t fingers;
            }ts_event;
        uint8_t dataread();    

    private:
        bool GSL1680_DEBUG_ERROR;
        bool GSL1680_DEBUG_INFO;
        void clear_reg();
        void reset();
        void load_fw();
        void startchip();
        void check_mem_data(uint8_t WAKE);
        void data_I2C_Write(uint8_t regAddr, uint8_t *val, uint16_t cnt);
        uint8_t data_I2C_Read( uint8_t regAddr, uint8_t * pBuf, uint8_t len );
};