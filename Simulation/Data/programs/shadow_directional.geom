 ފ         #     �                GLSL.std.450              	       main    �   �     T                                              �   
 GL_GOOGLE_cpp_style_line_directive       main     
    project_depth_linear(f1;f1;f1;    	   z     
   n        f        light_descriptor             position            radius          emittance           type            sampler_idx         effective_range_or_directional_distance         polygonal_light_points_and_offset_or_cascade_idx            _unused0     	       transformed_position         	   _unused1         light_get_cascade_descriptor_idx(struct-light_descriptor-vf3-f1-vf3-u1-u1-f1-u1-f1-vf3-f11;      ld   	    drawid_to_lightid_ttl_entry          data         translate_drawid_to_light_idx(struct-drawid_to_lightid_ttl_entry-u11;        entry     #   transform(vf4;mf34;   !   v     "   M    	 0   process(i1;u1;vf3[3];f1;      ,   cascade   -   cascade_idx   .   vertices      /   f    
 4   conversion_matrix_rgb_to_xyz     
 B   conversion_matrix_xyz_to_rgb      j   n     �   j     �   gl_Layer      �   z     �   gl_PerVertex      �       gl_Position   �      gl_PointSize      �      gl_ClipDistance   �      gl_CullDistance   �         �   param       param       param       sproj_instance_id       vs_out          instanceIdx        drawIdx     vin     draw_id     light_idx    	   drawid_to_lightid_ttl_entry         data        d_drawid_to_lightid_ttl         entries  
   d_drawid_to_lightid_ttl_data            ttl           "  param     )  ld    *  light_descriptor      *      position      *     radius    *     emittance     *     type      *     sampler_idx   *     effective_range_or_directional_distance   *     polygonal_light_points_and_offset_or_cascade_idx      *     _unused0     	 *     transformed_position      *  	   _unused1      ,  light_binding     ,      light_buffer      .        L  l     O  u     Q  gl_PerVertex      Q      gl_Position   Q     gl_PointSize      Q     gl_ClipDistance   Q     gl_CullDistance  	 Q     gl_SecondaryPositionNV   	 Q     gl_PositionPerViewNV      T  gl_in     ]  v     e  n     r  cascade_idx   s  param     v  cascade   ~  z_far       light_cascade_data          M          recp_vp        proj_eye_dist          proj_far_clip    	 �  light_cascades_descriptor     �      cascades      �  light_cascades_binding    �      light_cascades    �        �  M     �  j     �  vertices      �  param     �  param     �  param     �  param     �  param     �  param     �  dual_quaternion   �      real      �     dual     
 �  view_transform_buffer_struct      �      view_transform   	 �     inverse_view_transform    �     eye_position     
 �  view_transform_buffer_binding    	 �      view_transform_buffer     �       
 �  proj_transform_buffer_struct      �      proj_xywz     �     backbuffer_size   �     tan_half_fovy     �     aspect   
 �  proj_transform_buffer_binding    	 �      proj_transform_buffer     �       
 �  atmospheric_optical_length_lut   
 �  atmospheric_mie0_scattering_lut   �  atmospheric_ambient_lut  	 �  atmospheric_scattering_lut    �  atmospherics_descriptor   �      center_radius    	 �     scattering_coefficients  
 �     mie_absorption_coefficient    �     phase     �     Hm    �     Hr    �     minus_one_over_Hm     �     minus_one_over_Hr     �     Hm_max    �  	   Hr_max   
 �  atmospherics_descriptor_binding   �      atmospherics_descriptor_data      �        �  ltc_element   �      data     
 �  shaped_lights_points_binding      �      ltc_points    �       	 �  light_list_counter_binding    �      ll_counter    �        �  light_list_binding    �      ll    �       	 �  cascades_depths_uniform_t     �      cascades_depths   �      G  �      	   H  �              H  �            H  �            H  �            G  �      H           H          G       H        #       G          H           H           H        #       G          H           H           H        #       G       G    "       G    !       H  *         H  *      #       H  *        H  *     #      H  *        H  *     #      H  *        H  *     #      H  *        H  *     #       H  *        H  *     #   $   H  *        H  *     #   (   H  *        H  *     #   ,   H  *        H  *     #   0   H  *  	      H  *  	   #   <   G  +     @   H  ,         H  ,      #       G  ,     G  .  "      G  .  !      H  Q             H  Q           H  Q           H  Q           H  Q        �  H  Q        �  G  Q     H           H        #       H              H       #   0   H       #   8   H       #   <   G  �     @   H  �         H  �         H  �      #       G  �     �  H  �         H  �         H  �      #       G  �     G  �  "      G  �  !      H  �      #       H  �     #      H  �         H  �         H  �      #       H  �        H  �        H  �     #       H  �        H  �        H  �     #   @   H  �         H  �         H  �      #       G  �     G  �  "      G  �  !       H  �         H  �         H  �      #       H  �        H  �        H  �     #      H  �        H  �        H  �     #      H  �        H  �        H  �     #      H  �         H  �         H  �      #       G  �     G  �  "      G  �  !      G  �  "      G  �  !      G  �  "      G  �  !      G  �  "      G  �  !      G  �  "      G  �  !      H  �         H  �         H  �      #       H  �        H  �        H  �     #      H  �        H  �        H  �     #       H  �        H  �        H  �     #   $   H  �        H  �        H  �     #   (   H  �        H  �        H  �     #   ,   H  �        H  �        H  �     #   0   H  �        H  �        H  �     #   4   H  �        H  �        H  �     #   8   H  �  	      H  �  	      H  �  	   #   <   H  �         H  �         H  �      #       G  �     G  �  "      G  �  !      H  �         H  �         H  �      #       G  �        H  �         H  �         H  �      #       G  �     G  �  "      G  �  !      H  �         H  �      #       G  �     G  �  "      G  �  !      G  �        H  �         H  �      #       G  �     G  �  "      G  �  !      G  �        H  �      #       G  �     G  �  "            !                             !                                                                                        !                               !                                                         !                 %             &      %      '         +     (        )      (      *      )   !  +      &   '   *        2            3      2   ;  3   4      +     5   
-�>+     6   ��>+     7   ��8>,     8   5   6   7   +     9   m�Y>+     :   �7?+     ;   W͓=,     <   9   :   ;   +     =   Vb�<+     >   v�=+     ?   Bs?,     @   =   >   ?   ,  2   A   8   <   @   ;  3   B      +     C   dO@+     D   a�Ŀ+     E   '@��,     F   C   D   E   +     G   !x�+     H   p �?+     I   �6*=,     J   G   H   I   +     K   ��c=+     L   6�P�+     M   �U�?,     N   K   L   M   ,  2   O   F   J   N   +  %   Y      +  %   ^       +  %   a      +     k     zD  l   +     m       +     p     �?+  %   t      +  %   {      +     �     ��+     �      +     �      +  %   �         �      %   ;  �   �      +     �   w�+2  �      �     �         �   �      �      �   ;  �   �        �            �            �                        %            (             ;                 %                     +                                              ;            #         *                                  +  *    ,  +     -     ,  ;  -  .        0     *  +  %   <     +  %   ?     +  %   D     +  %   G     +  %   J  	     P     �     Q        �   �      P    R  Q  (      S     R  ;  S  T        U        +     m             �         +     �       �    �    �  �    �  �      �  �     �     �  ;  �  �        �           �        +     �     +     �  p   +     �     +     �      +     �  0   +     �     +     �     +     �     +     �     +     �  "   +     �  &   +     �  6   +     �  �I@+     �  ��?+     �  �
�?+     �  �I?+     �  |� ?+     �  ���>+     �  ��"?+     �  Evt?+     �  ���?+     �  ��?+     �  ��@+     �  ��A+     �  �IA+     �  �S{A+     �  T�-@+     �  �Z�>+     �  ���.+     �  �O
+     �    �  �          �  �  �       �  �     �     �  ;  �  �       �          �     �          �  �     �     �  ;  �  �      	 �                            �  �     �      �  ;  �  �       	 �                             �  �     �      �  ;  �  �      ;  �  �      ;  �  �        �                                  �  �     �     �  ;  �  �       �  �    �  �    �  �     �     �  ;  �  �     +  %   �     +  %   �       �        �     �  ;  �  �       �       �  �     �     �  ;  �  �     +     �   @�E  �     �    �  �     �     �  ;  �  �     6               �     ;  &        ;  '        ;  '        ;     "     ;     )     ;  �   L     ;  �   O     ;  �   ]     ;  �   e     ;  '   r     ;     s     ;  &   v     ;     ~     ;     �     ;  &   �     ;  *   �     ;     �     ;     �     ;  &   �     ;  '   �     ;  *   �     ;     �     >  4   A   >  B   O   A        ^   ^   =  %       >      A        ^   t   =         >      =          =  %   !    A  #  $    ^      ^   !  =    %  $  Q     &  %      A  '   '  "  ^   >  '  &  9     (     "  >    (  =     /    A  0  1  .  ^   /  =  *  2  1  Q     3  2      A  �   4  )  ^   >  4  3  Q     5  2     A     6  )  t   >  6  5  Q     7  2     A  �   8  )  {   >  8  7  Q     9  2     A  '   :  )  �   >  :  9  Q     ;  2     A  '   =  )  <  >  =  ;  Q     >  2     A     @  )  ?  >  @  >  Q     A  2     A  '   B  )  Y   >  B  A  Q     C  2     A     E  )  D  >  E  C  Q     F  2     A  �   H  )  G  >  H  F  Q     I  2  	   A     K  )  J  >  K  I  A  �   M  )  G  =     N  M  >  L  N  A  U  V  T  {   ^   =     W  V  O     X  W  W            A  U  Y  T  t   ^   =     Z  Y  O     [  Z  Z            �     \  X  [  >  O  \  A  U  ^  T  ^   ^   =     _  ^  O     `  _  _            A  U  a  T  t   ^   =     b  a  O     c  b  b            �     d  `  c  >  ]  d  =     f  O  =     g  ]       h     D   f  g  >  e  h  =     i  e  =     j  L       k  j  �     l  i  k  �  l   n  l  m  �  p      �  n  o  p  �  o  �  �  p  =     t  )  >  s  t  9     u     s  >  r  u  >  v  ^   �  w  �  w  �  y  z      �  {  �  {  =  %   |  v  �  l   }  |  Y   �  }  x  y  �  x  =     �  r  =  %   �  v  A 	 �  �  �  ^   �  ^   �  �   =     �  �  >  ~  �  =     �  r  =  %   �  v  A 	 �  �  �  ^   �  ^   �  ^   =     �  �  >  �  �  >  �  ^   �  �  �  �  �  �  �      �  �  �  �  =  %   �  �  �  l   �  �  �   �  �  �  �  �  �  =  %   �  �  =  %   �  �  A  U  �  T  �  ^   =     �  �  >  �  �  =     �  �  >  �  �  9     �  #   �  �  A  �   �  �  �  >  �  �  �  �  �  �  =  %   �  �  �  %   �  �  t   >  �  �  �  �  �  �  =  %   �  v  >  �  �  =     �  r  >  �  �  =  )   �  �  >  �  �  =     �  ~  >  �  �  9     �  0   �  �  �  �  �  z  �  z  =  %   �  v  �  %   �  �  t   >  v  �  �  w  �  y  �  8  6               7     	   7     
   7        �     =     P      =     Q   	   �     R   P   Q   =     S      =     T   
   �     U   S   T   �     V   R   U   �  V   8  6               7        �     A  '   Z      Y   =     [   Z   �  [   8  6               7        �     A  '   _      ^   =     `   _   �     b   `   a   �  b   8  6     #           7     !   7     "   �  $   =     e   !   =     f   "   �     g   e   f   �  g   8  6     0       +   7  &   ,   7  '   -   7  *   .   7     /   �  1   ;     j      ;  &   �      ;     �      ;     �      ;          ;          >  j   k   A     n   .   ^   m   =     o   n   �  l   q   o   p   �  s       �  q   r   s   �  r   A     u   .   t   m   =     v   u   �  l   w   v   p   �  s   �  s   �  l   x   q   1   w   r   �  z       �  x   y   z   �  y   A     |   .   {   m   =     }   |   �  l   ~   }   p   �  z   �  z   �  l      x   s   ~   y   �  l   �      �  �       �  �   �   �   �  �   A     �   .   ^   m   =     �   �   �  l   �   �   �   �  �       �  �   �   �   �  �   A     �   .   t   m   =     �   �   �  l   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �       �  �   �   �   �  �   A     �   .   {   m   =     �   �   �  l   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �   �  �   �  l   �      z   �   �   �  l   �   �   �  �       �  �   �   �   �  �   A     �   .   ^   �   =     �   �   �  l   �   �   p   �  �       �  �   �   �   �  �   A     �   .   t   �   =     �   �   �  l   �   �   p   �  �   �  �   �  l   �   �   �   �   �   �  �       �  �   �   �   �  �   A     �   .   {   �   =     �   �   �  l   �   �   p   �  �   �  �   �  l   �   �   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  l   �   �   �  �       �  �   �   �   �  �   A     �   .   ^   �   =     �   �   �  l   �   �   �   �  �       �  �   �   �   �  �   A     �   .   t   �   =     �   �   �  l   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �       �  �   �   �   �  �   A     �   .   {   �   =     �   �   �  l   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  l   �   �   �  �       �  �   �   �   �  �   A     �   .   ^   �   =     �   �   =     �   /        �   �   �  l   �   �   �   �  �       �  �   �   �   �  �   A     �   .   t   �   =     �   �   =     �   /        �   �   �  l   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �       �  �   �   �   �  �   A     �   .   {   �   =     �   �   =     �   /        �   �   �  l   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �   �  �   �  l   �   �   �   �   �   �  �       �  �   �   �   �  �   �  �  �   >  �   ^   �  �   �  �   �  �   �       �  �   �  �   =  %   �   �   �  l   �   �   �   �  �   �   �   �  �   =  %   �   ,   =     �   -   |  %   �   �   �  %   �   �   Y   �  %   �   �   �   >  �   �   =     �   j        �   �   �     �   �   �   =  %   �   �   A     �   .   �   �   =     �   �        �      %   �   �   >  �   �   =  %   �   �   A  �   �   .   �   =     �   �   O  �   �   �   �          A  �   �   �   ^   =     �   �   O 	    �   �   �               >  �   �   =        �   >  �      =       j   >      =       /   >      9          �       A      �   ^   �   >      A      �   ^   (   >    p   �  �  �   �  �   =  %   	  �   �  %   
  	  t   >  �   
  �  �   �  �   �  �  8  