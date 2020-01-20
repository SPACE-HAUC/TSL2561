#include "tsl2561.h"

/*
TSL2561 DEVICE INITIALIZATION 
*/

int tsl2561_init(tsl2561 *dev, uint8_t s_address)
{
    int32_t dev_id = 0;
    tsl2561_command m_con;
    uint8_t write_data = 0;

    // Create the file descriptor handle to the device
    dev->fd = open(I2C_BUS, O_RDWR);
    if (dev->fd < 0)
    {
        perror("[ERROR] Could not create a file descriptor.");
        return -1;
    }

    // Configure file descriptor handle as a slave device w/input address
    if (ioctl(dev->fd, I2C_SLAVE, s_address) < 0)
    {
        perror("[ERROR] Could not assign device.");
        return -1;
    }

    // Power the device - write to control register

    // 1) Configure command register
    m_con.cmd = 1;
    m_con.clear = 0;
    m_con.word = 0;
    m_con.block = 0;
    m_con.address = TSL2561_REGISTER_CONTROL;

    // 2) Write to the control register
    write_data = 0x03;
    uint8_t cmd[2] ;
    cmd[0] = m_con.raw ; cmd[1] = write_data ;
    if (write(dev->fd, cmd, 2) < 2)
    {
        perror("[ERROR] Could not power on the sensor.");
        return -1;
    }
    // printf("[DEBUG] TSL2561 waiting to proceed...\n") ;
    // char c;
    // do
    // {
    //     printf("Press enter key to continue...");
    //     scanf("%c", &c);
    // } while (c != '\n');
    usleep(10000);
    // Device is powered --> verify device is TSL2561 sensor - read from ID reg

    // 1) Configure the command register
    // m_con.cmd = 1;
    // m_con.clear = 0;
    // m_con.word = 0;
    // m_con.block = 0;
    // m_con.address = TSL2561_REGISTER_ID;
    cmd[0] = 0x8a ;
    // 2) Initiate the read command and read byte from ID register
    dev_id = write(dev->fd, cmd,1);
    usleep(100);
    dev_id = read(dev->fd, cmd, 1) ;
    if (dev_id < 1)
    {
        perror("[ERROR] Could not read device ID.");
        return -1;
    }

    // Extract the part number and the revision number
    uint8_t partno = (cmd[0] & 0xF0) >> 4;
    uint8_t revno = (cmd[0] & 0x0F);

    printf("PARTNO: [%X], REVNO: [%04X]\n", partno, revno);
    cmd[0] = 0x80 ;
    dev_id = write(dev->fd, &(m_con.raw),1);
    if ((dev_id = read(dev->fd, cmd,1)) < 0)
    {
        perror("[ERROR] Could not read device config.");
        return -1;
    }
    printf("[DEBUG] TSL2561 Init: Read config register: 0x%x\n", cmd[0]) ;
    if ((uint8_t)cmd[0] & 0x03)
    {
        printf("Initialization failed: 0x%x\n\n", 0x000000ff & (uint8_t)cmd[0]);
    }
    return 1;
}

int tsl2561_configure(tsl2561 *dev)
{
    // tsl2561_command m_con;
    // uint8_t timing_conf = TSL2561_INTEGRATIONTIME_13MS;
    // // 1) Configure command register
    // m_con.cmd = 1;
    // m_con.clear = 0;
    // m_con.word = 0;
    // m_con.block = 0;
    // m_con.address = TSL2561_REGISTER_TIMING;

    // if (i2c_smbus_write_byte_data(dev->fd, m_con.raw, timing_conf) < 0)
    // {
    //     perror("[ERROR] Could not change the integration time.");
    //     return -1;
    // }
    uint8_t cmd[2] ;
    cmd[0] = 0x81 ; cmd[1] = 0x00 ;
    if ( write(dev->fd, cmd, 2) < 2)
    {
        perror("[ERROR] Could not write the integration time register.");
    }
    if ( read(dev->fd, cmd, 1) < 1)
    {
        perror("[ERROR] Could not read the integration time register.");
    }
    printf("[DEBUG] TSL2561 config timing reg: 0x%x\n", cmd[0]);
    return 1;
}

int tsl2561_write(tsl2561 *dev, uint8_t reg_addr, uint8_t data)
{
    tsl2561_command m_con;

    // Configure the command register with the input register address
    m_con.cmd = 1;
    m_con.clear = 0;
    m_con.word = 0;
    m_con.block = 0;
    m_con.address = reg_addr;

    // Write the input data to the target register
    if (i2c_smbus_write_byte_data(dev->fd, m_con.raw, data) < 0)
    {
        perror("[ERROR] Could not write to device.");
        return -1;
    }

    return 1;
}

int tsl2561_read_block_data(tsl2561 *dev, uint8_t *data)
{
    // tsl2561_command m_con;

    // // Configure command register for block reading of all the data registers
    // m_con.cmd = 1;
    // m_con.clear = 0;
    // m_con.word = 0;
    // m_con.block = 1;
    // m_con.address = TSL2561_BLOCK_READ;

    // Perform block reading of all 4 data registers - 4 bytes read
    if (i2c_smbus_read_i2c_block_data(dev->fd, 0x9b, 4, data) < 0)
    {
        perror("[ERROR] Could not perform a block read from the data registers.");
        return -1;
    }

    // Return the read data
    return 1;

    // You should not allocate memory in a function call and return a pointer to that to the caller,
    // this is a very easy way to create a memory leak.
}

int tsl2561_read_word_data(tsl2561*dev , uint8_t * data)
{
    // Read the Ch0 register
    int ch0 = 0x00 ;
    if ( (ch0 = i2c_smbus_read_word_data(dev->fd, 0xac)) < 0)
    {
        perror("[ERROR] Could not perform a word read from the ch0.");
        return -1;
    }
    *((uint16_t*)data) = 0x0000ffff & ch0 ;
    printf("[DEBUG] Read Ch0: 0x%x\n", ch0);
    if ( (ch0 = i2c_smbus_read_word_data(dev->fd, 0xae)) < 0)
    {
        perror("[ERROR] Could not perform a word read from the ch1.");
        return -1;
    }
    *((uint16_t*)&(data[2])) = 0x0000ffff & ch0 ;
    printf("[DEBUG] Read Ch0: 0x%x\n", ch0);
    return 1 ;
}

int tsl2561_read_byte_data(tsl2561* dev, uint8_t * data)
{
    uint8_t cmd = 0x8c ; int status = 1 ;
    for ( int i = 0 ; i < 4 ; i++ )
    {
        uint8_t tmp = 0x00 ;
        int ch = i2c_smbus_read_byte_data(dev->fd, cmd) ;
        cmd++ ;
        if ( ch < 0 )
        {
            perror("[ERROR] Could not read byte data") ;
            status = 0 ;
        }
        data[i] = ch ;
        printf("[DEBUG] TSL2561 read byte: %d 0x%x -> 0x%x\n", ch , cmd - 1, tmp ) ;
    }
    return status ;
}

int tsl2561_read_i2c_data(tsl2561* dev, uint8_t * data)
{
    char cmd = 0x9b ;
    write(dev->fd, &cmd, 1) ;
    read(dev->fd, data, 4) ;
    return 1 ;
}

int tsl2561_read_config(tsl2561 *dev, uint8_t *data)
{
    tsl2561_command m_con;
    int32_t configuration = 0;

    // Configure command register for block reading of all the data registers
    m_con.cmd = 1;
    m_con.clear = 0;
    m_con.word = 0;
    m_con.block = 0;
    m_con.address = TSL2561_REGISTER_TIMING;

    // Perform block reading of all 4 data registers - 4 bytes read
    configuration = i2c_smbus_read_byte_data(dev->fd, m_con.raw);
    if (configuration < 0)
    {
        perror("[ERROR] Could not read from the data registers.");
        return -1;
    }

    // Return the read data
    *data = (configuration & 0x000000FF);
    return 1;
}

void tsl2561_destroy(tsl2561 *dev)
{
    close(dev->fd);
    free(dev);
}