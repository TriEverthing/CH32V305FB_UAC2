#HSE frequency 24MHz
HSE_Freq = ( 1 + 0e-6 ) * 16e6 ;
#I2S Audio WS frequency 176.4KHz 
WS_Freq = 352.8e3 ;
#I2S Data Width
Data_Witdh = 32 ;
#I2S Master Clocks Output Enable
Master_Clocks = 0 ;
#accuracy
v_accuracy = 0.5;
#arryay
import numpy as np
#pll3 divide
PLL3_DIV = np.arange( 2 , 17 , 1 );
#print(PLL3_DIV.shape);
#pll3 MUL
PLL3_MUL = np.array([2.5, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 12.5, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0, 20.0]);
#print(PLL3_MUL.shape);
#I2S_DIV
I2S_DIV = np.arange( 4 , 512 , 1 );
#print(I2S_DIV.shape);

PLL3_DIV_3D = 1.0 / PLL3_DIV.reshape( 15 , 1 , 1 );
#print(PLL3_DIV_3D.shape);
PLL3_MUL_3D = PLL3_MUL.reshape( 1 , 16 , 1 );
#print(PLL3_MUL_3D.shape);
I2S_DIV_3D = 1.0 / I2S_DIV.reshape( 1 , 1 , 508 );
#print(I2S_DIV_3D.shape);

#print(PLL3_DIV.shape);
i2s_fac = PLL3_DIV_3D * PLL3_MUL_3D * I2S_DIV_3D * 2 ;
#print(i2s_fac.shape);

if  Master_Clocks :
    freq_result = HSE_Freq * i2s_fac / 256 ;
else :
    freq_result = HSE_Freq * i2s_fac / Data_Witdh / 2 ;

freq_error = abs(( freq_result - WS_Freq ) / WS_Freq ) ;
freq_bool = freq_error < ( v_accuracy / 100.0 ) ;
#traverse
for index, vaule in np.ndenumerate(freq_bool):
    if vaule :
        #print( index , vaule );
        pll3_div = PLL3_DIV[ index[0] ] ;
        pll3_mul = PLL3_MUL[ index[1] ] ;
        i2s_div = I2S_DIV[ index[2] ] ;
        vco_freq = HSE_Freq * pll3_mul / pll3_div ;
        pll_in_freq = HSE_Freq / pll3_div ;
        if  Master_Clocks :
            freq_t =  vco_freq * 2 / i2s_div / 256 ;
        else :
            freq_t =  vco_freq * 2 / i2s_div / Data_Witdh / 2 ;
        freq_t = np.around(freq_t,5)
        if vco_freq <= 150000000 and vco_freq >= 60000000 and pll_in_freq <= 25000000 and pll_in_freq >= 3000000:
            print( "pll3_div=", pll3_div , ", pll3_mul=" , pll3_mul , ", i2s_div=" , i2s_div , "Freq:", freq_t , "Accuracy=" ,np.around( (freq_t - WS_Freq ) / WS_Freq * 100 , 5 ), "%" )



#####


