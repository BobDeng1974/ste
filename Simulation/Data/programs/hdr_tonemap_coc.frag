 ފ         #     |             +        GLSL.std.450                     main    �   S  U               �   
 GL_GOOGLE_cpp_style_line_directive       main         XYZtoRGB(vf3;     
   xyz      xyYtoXYZ(vf3;        xyY      red_response(f1;         mesopic      hdr_bin(f1;f1;f1;        max_lum      min_lum      l        hdr_lum(f1;      l        tonemap(f1;      l     '   hdr_bloom(vf4;vf3;f1;     $   RGBL      %   XYZ   &   mesopic   *   hdr_tonemap(f1;   )   l    
 .   conversion_matrix_rgb_to_xyz     
 <   conversion_matrix_xyz_to_rgb      O   Y_y   X   XYZ   t   range     x   bin_size      |   r     �   x     �   min_lum   �   hdr_bokeh_parameters      �       lum_min   �      lum_max   �      focus    	 �   hdr_bokeh_parameters_buffer   �       params    �         �   max_lum   �   fbin      �   param     �   param     �   param     �   bin   �   frac      �   T     �   histogram_sums    �       histogram     �         �   toned_bin_start   �   toned_bin_end     �   toned_l   �   param     �   hdr_texel     �   hdr   �   gl_FragCoord        x     
  vision_properties_coord     vision_properties    
   hdr_vision_properties_texture       scotopic        mesopic     monochr   !  acuity    %  red_coef      &  param     )  l     *  param     -  param     5  XYZ   6  param     9  RGB   :  param     H  RGBL      S  rgbout    U  bloomout      V  param     X  param     Z  param     q  dual_quaternion   q      real      q     dual     
 r  view_transform_buffer_struct      r      view_transform   	 r     inverse_view_transform    r     eye_position     
 s  view_transform_buffer_binding    	 s      view_transform_buffer     u       
 w  proj_transform_buffer_struct      w      proj_xywz     w     backbuffer_size   w     tan_half_fovy     w     aspect   
 x  proj_transform_buffer_binding    	 x      proj_transform_buffer     z      H  �          H  �          H  �       #       H  �         H  �         H  �      #      H  �         H  �         H  �      #      H  �          H  �          H  �       #       G  �      G  �   "       G  �   !      G  �         H  �          H  �          H  �       #       G  �      G  �   "       G  �   !      G  �   "       G  �   !       G  �         G    "       G    !      G  S         G  U        H  q      #       H  q     #      H  r         H  r         H  r      #       H  r        H  r        H  r     #       H  r        H  r        H  r     #   @   H  s         H  s         H  s      #       G  s     G  u  "      G  u  !       H  w         H  w         H  w      #       H  w        H  w        H  w     #      H  w        H  w        H  w     #      H  w        H  w        H  w     #      H  x         H  x         H  x      #       G  x     G  z  "      G  z  !           !                                        !  	                     !           !                   !            "      !   !  #   !   "           ,            -      ,   ;  -   .      +     /   
-�>+     0   ��>+     1   ��8>,     2   /   0   1   +     3   m�Y>+     4   �7?+     5   W͓=,     6   3   4   5   +     7   Vb�<+     8   v�=+     9   Bs?,     :   7   8   9   ,  ,   ;   2   6   :   ;  -   <      +     =   dO@+     >   a�Ŀ+     ?   '@��,     @   =   >   ?   +     A   !x�+     B   p �?+     C   �6*=,     D   A   B   C   +     E   ��c=+     F   6�P�+     G   �U�?,     H   E   F   G   ,  ,   I   @   D   H     P           +  P   Q      +  P   T      +  P   Z       +     `     �?+     o   33�>+     z      C+     �       +     �   fff?  �   +     �   ���=+     �      A,  !   �   �   �   �   �     �            �   �   �        �   �      �      �   ;  �   �      +  �   �          �      �   +  �   �         �      �   +  P   �   �     �   P   �     �   �      �      �   ;  �   �      +  �   �         �      P    	 �                              �   �      �       �   ;  �   �          �      !   ;  �   �        �             �      +       �7�5+       ��A 	                                              ;          +  P   "     +     0  �~*?,  !   P  `   `   `   `      R     !   ;  R  S     ;  R  U     +     ]  �I@+     ^  ��?+     _  �
�?+     `  �I?+     a  |� ?+     b  ���>+     c  ��"?+     d  Evt?+     e  ���?+     f  ��?+     g  ��@+     h  ��A+     i  �IA+     j  �S{A+     k  T�-@+     l  �Z�>+     m  ���.+     n  �O
+     o    �+  �   p  �     q  !   !     r  q  q  !     s  r     t     s  ;  t  u       v  P        w  !   v          x  w     y     x  ;  y  z     +     {     A6               �     ;     �      ;          ;     
     ;  "        ;          ;          ;          ;     !     ;     %     ;     &     ;     )     ;     *     ;     -     ;     5     ;     6     ;     9     ;     :     ;  "   H     ;  "   V     ;     X     ;     Z     >  .   ;   >  <   I   =  �   �   �   =  !      �   O  �                  n        d  �     �   _  !            �   O                     >  �     A       �   Q   =     	    >    	  =         �           �           >  
    =        =       
  W  !         >      A         Z   =         >      A         T   =         >      A         Q   =          >       A     #    "  =     $  #  >  !  $  =     '    >  &  '  9     (     &  >  %  (  =     +    >  *  +  9     ,     *  >  )  ,  =     .  )  >  -  .  9     /  *   -  =     1         2     .   `   0  1  �     3  /  2  A     4  �   Q   >  4  3  =     7  �   >  6  7  9     8     6  >  5  8  =     ;  5  >  :  ;  9     <     :  >  9  <  =     =  %  A     >  9  Z   =     ?  >  �     @  ?  =  A     A  9  Z   >  A  @  =     B  9  =     C  5  O     D  C  C           =     E    P     F  E  E  E       G     .   B  D  F  >  9  G  =     I  9  A     J  5  T   =     K  J  Q     L  I      Q     M  I     Q     N  I     P  !   O  L  M  N  K    !   Q     +   O  �   P  >  H  Q  =  !   T  H  >  S  T  =  !   W  H  >  V  W  =     Y  5  >  X  Y  =     [    >  Z  [  9  !   \  '   V  X  Z  >  U  \  �  8  6            	   7     
   �     =     J   
   =  ,   K   <   �     L   J   K   �  L   8  6            	   7        �     ;     O      ;     X      A     R      Q   =     S   R   A     U      T   =     V   U   �     W   S   V   >  O   W   =     Y   O   A     [      Z   =     \   [   �     ]   Y   \   A     ^   X   Z   >  ^   ]   =     _   O   A     a      Z   =     b   a   �     c   `   b   A     d      T   =     e   d   �     f   c   e   �     g   _   f   A     h   X   Q   >  h   g   A     i      Q   =     j   i   A     k   X   T   >  k   j   =     l   X   �  l   8  6               7        �     =     p           q      .   `   o   p   �  q   8  6               7        7        7        �     ;     t      ;     x      ;     |      =     u      =     v      �     w   u   v   >  t   w   =     y   t   �     {   y   z   >  x   {   =     }      =     ~      �        }   ~   =     �   x   �     �      �   >  |   �   =     �   |        �      +   �   �   z   �  �   8  6               7        �     =     �      �     �   �   `        �         �   �  �   8  6               7        �      =     �      �  �   8  6  !   '       #   7  "   $   7     %   7     &   �  (   ;     �      A     �   %   T   =     �   �   �  �   �   �   �   �  �       �  �   �   �   �  �   A     �   %   T   =     �   �   �     �   �   �   �     �   �   �        �         �   �   =     �   &   �     �   `   �   �     �   �   �   >  �   �   =  !   �   $   O     �   �   �             =     �   �   Q     �   �       Q     �   �      Q     �   �      P  !   �   �   �   �   �   �  �   �  �   �  �   �  �     !   �   �  �   8  6     *          7     )   �  +   ;     �      ;     �      ;     �      ;     �      ;     �      ;     �      ;  �   �      ;     �      ;     �      ;     �      ;     �      ;     �      ;     �      ;     �      A  �   �   �   �   �   =  �   �   �   |     �   �   >  �   �   A  �   �   �   �   �   =  �   �   �   |     �   �   >  �   �   =     �   �   >  �   �   =     �   �   >  �   �   =     �   )   >  �   �   9     �      �   �   �   >  �   �   =     �   �   n  �   �   �   >  �   �   =     �   �        �      
   �   >  �   �   A  �   �   �   �   �   =  P   �   �   p     �   �   >  �   �   =  �   �   �   �  �   �   �   �   �  �       �  �   �   �   �  �   =  �   �   �   �  �   �   �   �   A  �   �   �   �   �   =  P   �   �   p     �   �   =     �   �   �     �   �   �   >  �   �   �  �   �  �   >  �   �   �  �   �  �   =     �   �   >  �   �   =  �   �   �   A  �   �   �   �   �   =  P   �   �   p     �   �   =     �   �   �     �   �   �   >  �   �   =     �   �   =     �   �   =     �   �        �      .   �   �   �   >  �   �   =     �   �   >  �   �   9     �      �   �  �   8  