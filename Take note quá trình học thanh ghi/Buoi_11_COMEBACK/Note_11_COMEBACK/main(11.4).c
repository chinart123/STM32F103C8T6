#include <stm32f103c8t6.h>

/*
@@ CÚ PHÁP KIỂM TRA TRONG CỬA SỔ WATCH (IAR C-SPY):
   - "(unsigned char*)data1;20" : Hiển thị 20 phần tử liên tiếp từ mảng nguồn.
   - "(unsigned char*)data2;20" : Hiển thị 20 phần tử liên tiếp từ mảng đích.
   
   Mục đích: Chứng minh tư duy vòng lặp "for (i += 2)" bằng cách kiểm tra xem
   các phần tử ở vị trí LẺ (index 1, 3, 5, 7...) của data2 có giữ nguyên là số 0 không,
   trong khi các vị trí CHẴN nhận được dữ liệu nhảy cóc từ data1.
*/

unsigned char data1[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
unsigned char data2[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

volatile unsigned char count = 0; // Biến đếm số lượt nhảy (tương đương biến i trong vòng for)

void main()
{
  RCC.AHB_ENR.BITS.DMA1 = 1; // Cấp xung clock cho bộ điều khiển DMA1
  
  // Kích hoạt ngắt toàn cục cho kênh DMA1 Channel 1 tại bộ quản lý ngắt NVIC
  NVIC.ISER.BITS.DMA1_CN1 = 1;

  DMA1.Channel_1.CMAR = (unsigned long)&data2; // Địa chỉ ĐÍCH ĐẾN ban đầu (ô nhớ số 0)
  DMA1.Channel_1.CPAR = (unsigned long)&data1; // Địa chỉ NGUỒN ban đầu (ô nhớ số 0)

  DMA1.Channel_1.CNDTR = 1;  // Số lượng dữ liệu truyền mỗi đợt = 1 byte
  
  // Cấu hình thanh ghi CCR:
  // BIT14 (MEM2MEM) : Cho phép truyền từ RAM sang RAM
  // BIT1  (TCIE)    : Bật ngắt khi truyền xong 1 byte (Transfer Complete Interrupt Enable)
  // BIT0  (EN)      : Kích hoạt bật kênh DMA bắt đầu chạy lượt đầu tiên
  // KHÔNG bật BIT7 (MINC) và BIT6 (PINC) để phần cứng cố định địa chỉ, nhường quyền tăng địa chỉ cho CPU
  DMA1.Channel_1.CCR.REG = BIT14 | BIT1 | BIT0;
  
  while (1)
  {
    // Vòng lặp chính trống. CPU rảnh tay làm việc khác, chỉ xử lý địa chỉ khi có ngắt DMA.
  }
}

// Trình phục vụ ngắt DMA1 Channel 1 (Tên hàm đồng bộ theo vector table trong startup)
void DMA1_CN1_IRQHandler(void)
{
  // Kiểm tra cờ ngắt Hoàn thành truyền (TCIF1 - Bit 1) trong thanh ghi trạng thái ISR
  if ((DMA1.ISR.REG & BIT1) != 0) 
  {
    DMA1.Channel_1.CCR.REG &= ~BIT0; // TẮT DMA (EN = 0) tạm thời để có quyền thay đổi cấu hình thanh ghi

    count++; // Tăng biến đếm lượt truyền
    if (count < 10) // Thực hiện vòng lặp nhảy 10 đợt (tổng cộng dịch 20 byte)
    {
      DMA1.Channel_1.CPAR += 2; // TƯ DUY VÒNG LẶP FOR: Ép con trỏ nguồn nhảy cóc 2 bước
      DMA1.Channel_1.CMAR += 2; // TƯ DUY VÒNG LẶP FOR: Ép con trỏ đích nhảy cóc 2 bước
      
      DMA1.Channel_1.CNDTR = 1;       // Nạp lại số lượng truyền cho lượt tiếp theo là 1 byte
      DMA1.Channel_1.CCR.REG |= BIT0; // BẬT LẠI DMA (EN = 1) để kích hoạt lượt truyền mới
    }
    
    DMA1.IFCR.REG |= BIT1; // Ghi 1 vào bit CTCIF1 để xóa cờ ngắt, sẵn sàng cho lần ngắt kế tiếp
  }
}
