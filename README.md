# openBVE-TASC-ATC

This Plugin is developed to operate on openBVE and does not guarantee that it will work on other BVE.

This Plugin is a plug-in that integrates TASC and analog ATC.


### 한국어

본 플러그인은 openBVE에서의 구동을 상정하고 개발 되었으며, BVE Trainsim 에서의 구동은 보장하지 않습니다.

본 플러그인은 TASC와 아날로그 방식의 ATC 코드가 통합된 플러그인 입니다.

## 적용 방법

### ATC

ATC 기능은 Section 구문을 통해 작동됩니다.

루트에 Section 구간을 설치하면 Section의 Signal 상태에 따라 속도가 변동됩니다. (Signal의 숫자가 낮아질수록 ATC 제한속도도 낮아짐)
