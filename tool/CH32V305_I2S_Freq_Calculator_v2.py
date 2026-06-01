#HSE frequency 24MHz
HSE_Freq = ( 1 + 0e-6 ) * 24e6 ;
#I2S Audio WS frequency 384KHz 
WS_Freq = 192e3 ;
#I2S Data Width
Data_Witdh = 32 ;
#I2S Master Clocks Output Enable
Master_Clocks = 1 ;
#accuracy
v_accuracy = 0.5 ;
#arryay
import numpy as np
#PLL2 divide
PLL2_DIV = np.arange( 1 , 17 , 1 );
#print(PLL2_DIV.shape);
#PLL2 MUL
PLL2_MUL = np.array([2.5, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 12.5, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0, 20.0]);
#print(PLL2_MUL.shape);
#PLL1 divide
PLL1_DIV = np.arange( 1 , 17 , 1 );
#print(PLL1_DIV.shape);
#PLL1 MUL
PLL1_MUL = np.array([2.5, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 12.5, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0, 20.0]);
#print(PLL1_MUL.shape);
I2S_DIV = np.arange( 4 , 512 , 1 );
#print(I2S_DIV.shape);

PLL2_DIV_5D = 1.0 / PLL2_DIV.reshape( 16 , 1 , 1 , 1 , 1 );
#print(PLL2_DIV_5D);
PLL2_MUL_5D = PLL2_MUL.reshape( 1 , 16 , 1 , 1 , 1);
#print(PLL2_MUL_5D);
PLL1_DIV_5D = 1.0 / PLL2_DIV.reshape( 1 , 1 , 16 , 1 , 1);
#print(PLL1_DIV_5D);
PLL1_MUL_5D = PLL2_MUL.reshape( 1 , 1 , 1 , 16 , 1 );
#print(PLL1_MUL_5D);
I2S_DIV_5D = 1.0 / I2S_DIV.reshape( 1 , 1 , 1 , 1 , 508 );
#print(I2S_DIV_3D.shape);

PPL1_VCO_5D = PLL2_DIV_5D * PLL2_MUL_5D * PLL1_DIV_5D * PLL1_MUL_5D * I2S_DIV_5D ;
#print(PPL1_VCO_5D.shape);

if  Master_Clocks :
    freq_result = HSE_Freq * PPL1_VCO_5D / 256 ;
else :
    freq_result = HSE_Freq * PPL1_VCO_5D / Data_Witdh / 2 ;

freq_error = abs( freq_result - WS_Freq ) / WS_Freq * 100 ;
freq_bool = freq_error < v_accuracy ;
#traverse
for index, vaule in np.ndenumerate(freq_bool):
    if vaule :
        #print( index , vaule );
        pll2_div = PLL2_DIV[ index[0] ] ;
        pll2_mul = PLL2_MUL[ index[1] ] ;
        pll1_div = PLL2_DIV[ index[2] ] ;
        pll1_mul = PLL2_MUL[ index[3] ] ;
        i2s_div = I2S_DIV[ index[4] ] ;
        pll2_vco_freq = HSE_Freq * pll2_mul / pll2_div ;
        pll2_in_freq = HSE_Freq / pll2_div ;
        pll1_vco_freq = pll2_vco_freq * pll1_mul / pll1_div ;
        pll1_in_freq = pll2_vco_freq / pll1_div ;
        if  Master_Clocks :
            freq_t =  pll1_vco_freq / i2s_div / 256 ;
        else :
            freq_t =  pll1_vco_freq / i2s_div / Data_Witdh / 2 ;
        
        freq_t = np.around(freq_t,5);
        pll2_vco_freq = np.around(pll2_vco_freq,1);
        pll2_in_freq = np.around(pll2_in_freq,1);
        pll1_vco_freq = np.around(pll1_vco_freq,1);
        pll1_in_freq = np.around(pll1_in_freq,1);

        err_t = np.around( (freq_t - WS_Freq ) / WS_Freq * 100 , 5 );

        if pll2_vco_freq <= 150000000 and pll2_vco_freq >= 60000000 and pll2_in_freq <= 25000000 and pll2_in_freq >= 3000000 and \
            pll1_vco_freq <= 144000000 and pll1_vco_freq >= 18000000 and pll1_in_freq <= 25000000 and pll1_in_freq >= 3000000:
            print( "pll2_div=", pll2_div , ", pll2_mul=" , pll2_mul , \
                   ",pll1_div=", pll1_div , ", pll1_mul=" , pll1_mul , \
                   "\r\npll2_vco_freq=", pll2_vco_freq , "Hz , pll2_in_freq=" , pll2_in_freq , "Hz" , \
                   "\r\npll1_vco_freq=", pll1_vco_freq , "Hz , pll1_in_freq=" , pll1_in_freq , "Hz" , \
                  "\r\ni2s_div=" , i2s_div , ", Freq:", freq_t , ", Accuracy=" , err_t , "%" )



#####


