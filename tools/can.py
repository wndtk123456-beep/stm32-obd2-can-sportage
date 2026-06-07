import serial
import time
import sys
import serial.tools.list_ports

def find_stm32_port():
    """연결된 시리얼 포트 중 STM32 포트를 찾아 반환합니다."""
    STM_VID = 0x0483 
    ports = serial.tools.list_ports.comports()
    
    for port in ports:
        if port.vid == STM_VID:
            return port.device
            
        description = str(port.description).upper()
        if 'STM' in description or 'STLINK' in description or 'ST-LINK' in description:
            return port.device
    return None

# --- 실행 및 적용 ---
PORT = find_stm32_port()
if PORT is None:
    # 🚨 주의: 다른 프로그램에 넘길 때는 flush=True가 필수입니다!
    print("포트를 찾지 못했습니다.", flush=True) 
    sys.exit()  
else:
    print(f"발견한 포트 : {PORT}", flush=True)

BAUDRATE = 9600

try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=0.1)
    print(f"✅ {PORT} 포트 연결 성공! (속도: {BAUDRATE})", flush=True)
except Exception as e:
    print(f"포트 연결 실패: {e}", flush=True)
    sys.exit()

while True:
    try:
        # 보드에서 올라오는 한 줄의 데이터를 읽음
        raw_data = ser.readline() 
        ''' Shape: raw_data [bytes length] (1D Array, 바이트 단위의 날것의 데이터 배열) '''
        
        if len(raw_data) > 0:
            # 바이트 배열을 사람이 읽을 수 있는 문자열로 변환
            text = raw_data.decode('utf-8', errors='replace').strip()
            ''' Shape: text [1] (스칼라 문자열, 해독된 단일 문장) '''
            
            if text:
                # 🚨 가장 중요한 핵심: flush=True를 넣어야 대시보드가 즉시 알아먹고 바늘을 움직입니다!
                print(f"[수신] raw={raw_data}, decoded={text}", flush=True)
                
    except Exception as e:
        # 통신 중 오류가 나면 0.5초 쉬고 다시 시도
        time.sleep(0.5)
