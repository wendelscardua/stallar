#include "../src/constants.h"

#pragma rodata-name ("RODATA")

const unsigned char metasprite_0_data[]={

                                         - 3,-16,0x00,2,
                                         - 3,- 8,0x10,2,
                                         0x80

};

const unsigned char metasprite_1_data[]={

                                         - 3,-16,0x01,2,
                                         - 3,- 8,0x11,2,
                                         0x80

};

const unsigned char metasprite_2_data[]={

                                         - 3,-16,0x00,2|OAM_FLIP_H,
                                         - 3,- 8,0x10,2|OAM_FLIP_H,
                                         0x80

};

const unsigned char metasprite_3_data[]={

                                         - 3,-16,0x01,2|OAM_FLIP_H,
                                         - 3,- 8,0x11,2|OAM_FLIP_H,
                                         0x80

};

const unsigned char metasprite_4_data[]={

                                         - 3,-12,0x02,3,
                                         0x80

};

const unsigned char metasprite_5_data[]={

                                         0,-16,0x03,1|OAM_FLIP_H,
                                         - 8,-16,0x04,1|OAM_FLIP_H,
                                         0,- 8,0x13,1|OAM_FLIP_H,
                                         - 8,- 8,0x14,1|OAM_FLIP_H,
                                         0x80

};

const unsigned char metasprite_6_data[]={

                                         0,-16,0x05,1|OAM_FLIP_H,
                                         - 8,-16,0x06,1|OAM_FLIP_H,
                                         0,- 8,0x15,1|OAM_FLIP_H,
                                         - 8,- 8,0x16,1|OAM_FLIP_H,
                                         0x80

};

const unsigned char metasprite_7_data[]={

                                         - 8,-16,0x03,1,
                                         0,-16,0x04,1,
                                         - 8,- 8,0x13,1,
                                         0,- 8,0x14,1,
                                         0x80

};

const unsigned char metasprite_8_data[]={

                                         - 8,-16,0x05,1,
                                         0,-16,0x06,1,
                                         - 8,- 8,0x15,1,
                                         0,- 8,0x16,1,
                                         0x80

};

const unsigned char metasprite_9_data[]={

                                         0,-16,0x07,0|OAM_FLIP_H,
                                         - 8,-16,0x08,0|OAM_FLIP_H,
                                         0,- 8,0x17,0|OAM_FLIP_H,
                                         - 8,- 8,0x18,0|OAM_FLIP_H,
                                         0x80

};

const unsigned char metasprite_10_data[]={

                                          0,-16,0x09,0|OAM_FLIP_H,
                                          - 8,-16,0x0a,0|OAM_FLIP_H,
                                          0,- 8,0x19,0|OAM_FLIP_H,
                                          - 8,- 8,0x1a,0|OAM_FLIP_H,
                                          0x80

};

const unsigned char metasprite_11_data[]={

                                          - 8,-16,0x07,0,
                                          0,-16,0x08,0,
                                          - 8,- 8,0x17,0,
                                          0,- 8,0x18,0,
                                          0x80

};

const unsigned char metasprite_12_data[]={

                                          - 8,-16,0x09,0,
                                          0,-16,0x0a,0,
                                          - 8,- 8,0x19,0,
                                          0,- 8,0x1a,0,
                                          0x80

};

const unsigned char metasprite_13_data[]={

                                          -16,-32,0x0b,0,
                                          - 8,-32,0x0c,1,
                                          0,-32,0x0d,1,
                                          8,-32,0x0e,0,

                                          -16,-24,0x1b,0,
                                          - 8,-24,0x1c,1,
                                          0,-24,0x1d,1,
                                          8,-24,0x1e,0,

                                          -16,-16,0x0f,0,
                                          8,-16,0x1f,0,
                                          -16,- 8,0x0f,0,
                                          8,- 8,0x1f,0,
                                          0x80

};

const unsigned char* const metasprite_list[]={

                                              metasprite_0_data,
                                              metasprite_1_data,
                                              metasprite_2_data,
                                              metasprite_3_data,
                                              metasprite_4_data,
                                              metasprite_5_data,
                                              metasprite_6_data,
                                              metasprite_7_data,
                                              metasprite_8_data,
                                              metasprite_9_data,
                                              metasprite_10_data,
                                              metasprite_11_data,
                                              metasprite_12_data,
                                              metasprite_13_data

};
