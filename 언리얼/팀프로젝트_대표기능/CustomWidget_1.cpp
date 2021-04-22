#include "CustomWidget_1.h"
#include "Widgets/CustomButton.h"

// 위젯의 클래스를 이 클래스에 등록합니다. 그러면 위젯상의 객체와 상호작용을 할 수 있습니다.

void UCustomWidget_1::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯의 'ExitButton', 'CancelButton' 이라는 이름을 가진 버튼을 각각 'CB_1', 'CB_2' 의 변수로 만듭니다.
	CB_1 = Cast<UCustomButton>(GetWidgetFromName(TEXT("ExitButton")));
	CB_2 = Cast<UCustomButton>(GetWidgetFromName(TEXT("CancelButton")));

	// 버튼을 누르면 등록된 함수가 호출됩니다.
	CB_1->OnClicked.AddDynamic(this, &UCustomWidget_1::OnClickedCB_1);
	CB_2->OnClicked.AddDynamic(this, &UCustomWidget_1::OnClickedCB_2);
}

void UCustomWidget_1::OnClickedCB_1()
{
	UE_LOG(LogTemp, Log, TEXT("Press CB_1"));
}

void UCustomWidget_1::OnClickedCB_2()
{
	UE_LOG(LogTemp, Log, TEXT("Press CB_2"));
}