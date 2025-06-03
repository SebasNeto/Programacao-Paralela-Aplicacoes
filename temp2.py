valortradicional = 1121.0713
valorthreads= 136.9613
valorjulia = 359.0624
valormp = 80.2740
valorhalide = 0.1182

div1 = valortradicional / valorthreads

div2 = valortradicional / valorjulia

div3 = valortradicional / valormp

div4 = valortradicional / valorhalide

print(f"threads foi : {div1:.6f} mais rápido que tradicional")
print(f"julia foi : {div2:.6f} mais rápido que tradicional")
print(f"mp foi : {div3:.6f} mais rápido que tradicional")
print(f"halide foi : {div4:.6f} mais rápido que tradicional")