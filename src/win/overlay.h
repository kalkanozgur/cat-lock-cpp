#pragma once

namespace kk::win {

// Her monitörü siyah, tıklama geçiren katmanla karartır. alpha 0-255; 0 = katman yok.
// Tekrar çağrılırsa katmanları güncel monitör düzenine göre yeniden kurar.
void overlay_show(int alpha);
void overlay_hide();

}  // namespace kk::win
