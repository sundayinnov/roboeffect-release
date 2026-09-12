import numpy as np
from scipy.signal import remez

# 16kHz采样率，通带7kHz，阻带7.8kHz，64阶，60dB阻带衰减
h = remez(64,
          [0, 7000, 7800, 8000],
          [1, 0],
          weight=[1, 10],
          fs=16000)

# 归一化：直流增益=1
h = h / np.sum(h)

# 导出为框图 vector 格式
with open("fir_16k_7k5_lpf.txt", "w") as f:
    f.write("#vector\n")
    for v in h:
        f.write(f"{v:.10f}\n")

print("OK, %d coefficients" % len(h))
print("Saved to fir_16k_7k5_lpf.txt")
