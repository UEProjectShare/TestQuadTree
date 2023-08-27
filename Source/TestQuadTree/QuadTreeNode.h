#pragma once

#include "CoreMinimal.h"
#include "Battery.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

class QuadTreeNode : public TSharedFromThis<QuadTreeNode>
{
public:
	FVector Center; // 中心点
	FVector Extend; // 扩展尺寸
	bool bIsLeaf;    //是否是叶子节点
	int32 Depth = 0;
	int32 MaxCount = 4;

	TArray<ABattery*> Objs; 
	static UObject* WorldObject;
	bool bInRange;
	
	TSharedPtr<QuadTreeNode> Root;
	TArray<TSharedPtr<QuadTreeNode>> Child_Node;

	QuadTreeNode(const FVector Inenter, FVector InExtend, int32 InDepth, const TSharedPtr<QuadTreeNode>& InRoot = nullptr)
		: Center(Inenter)
		, Extend(InExtend)
		, Depth(InDepth)
	{
		Root = InRoot;
		bIsLeaf = true;
		Child_Node.Init(nullptr, 4);
		bInRange = false;
	}
	
	~QuadTreeNode()
	{
		Root = nullptr;
		Objs.Empty();
		Child_Node.Empty();
	}

	bool IsNotUsed()
	{
		return bIsLeaf && Objs.Num() <= 0;
	}

	//方形与圆形求交
	bool InterSection(FVector InCenter, float InRadian)
	{
		FVector v = InCenter - Center; //取相对原点
		float x = UKismetMathLibrary::Min(v.X, Extend.X); 
		x = UKismetMathLibrary::Max(x, -Extend.X);

		float y = UKismetMathLibrary::Min(v.Y, Extend.Y);
		y = UKismetMathLibrary::Max(y, -Extend.Y);
		return (x - v.X) * (x - v.X) + (y - v.Y) * (y - v.Y) <= InRadian * InRadian; //注意此时圆心的相对坐标
	}

	//点是否在本区域内
	bool InterSection(FVector InPoint) const
	{	
		return (InPoint.X >= Center.X - Extend.X &&
			InPoint.X <= Center.X + Extend.X &&
			InPoint.Y >= Center.Y - Extend.Y &&
			InPoint.Y <= Center.Y + Extend.Y);
	}

	//点是否在指定区域内
	static bool InterSection(FVector InPMin, FVector InPMax, FVector InPoint)
	{		
		return (InPoint.X >= InPMin.X &&
			InPoint.X <= InPMax.X &&
			InPoint.Y >= InPMin.Y &&
			InPoint.Y <= InPMax.Y);
	}
	
	//插入对象
	void InsertObj(ABattery* InObj)
	{
		Objs.Add(InObj);
		if (bIsLeaf && Objs.Num() <= MaxCount) //直接插入			
		{				
			return;
		}	

		float dx[4] = { 1, -1, -1, 1 };
		float dy[4] = { 1, 1, -1, -1 };
		
		//超过上限个数，创建子节点;或者不再是叶子节点
		bIsLeaf = false;
		for (ABattery*& Item : Objs)
		{
			for (int i = 0; i < 4; i++)
			{
				//四个象限
				FVector p = Center + FVector(Extend.X * dx[i], Extend.Y * dy[i], 0);
				FVector pMin = p.ComponentMin(Center);
				FVector pMax = p.ComponentMax(Center);
				if (InterSection(pMin, pMax, Item->GetActorLocation()))
				{
					if (!Child_Node[i].IsValid())
					{
						Root = Root.IsValid() ? Root : this->AsShared();
						Child_Node[i] = MakeShareable(new QuadTreeNode(pMin/2+pMax/2, Extend / 2, Depth + 1, Root));
					}
					Child_Node[i]->InsertObj(Item);
					//break; //确保只在一个象限内
				}
			}			
		}
		Objs.Empty(); //确保非叶子节点不存
	}

	// 绘制区域边界
	void DrawBound(float InTime = 0.02f, float InThickness = 2.0f) const
	{	
		if (WorldObject)
		{
			const FLinearColor DrawColor = bInRange ? FLinearColor::Green : FLinearColor::Red;
			const FVector DrawCenter = Center + (bInRange ? FVector(0, 0, 8) : FVector(0, 0, 5));
			UKismetSystemLibrary::DrawDebugBox(WorldObject, DrawCenter, Extend+FVector(0,0,1), DrawColor, FRotator::ZeroRotator, InTime, InThickness);
		}
	}

	// 判断电池是否在扫描器的范围类
	void TraceObjectInRange(AActor* InTraceActor, float InRadian)
	{
		FVector InCenter = InTraceActor->GetActorLocation();
		if (InterSection(InCenter, InRadian))
		{
			bInRange = true;
			if (bIsLeaf)
			{
				for (ABattery* Obj : Objs)
				{					
					InCenter.Z = Obj->GetActorLocation().Z;
					const bool bCanActive = FVector::Distance(InCenter, Obj->GetActorLocation()) <= InRadian;
					Obj->ActiveState(bCanActive, InTraceActor);
				}
			}
			else
			{
				for (TSharedPtr<QuadTreeNode>& Node : Child_Node)
				{
					if (Node.IsValid())
					{
						Node->TraceObjectInRange(InTraceActor, InRadian);
					}
				}
			}
		}
		else
		{
			TraceObjectOutRange(InCenter, InRadian);
		}
	}
	

	void TraceObjectOutRange(FVector InCenter, float InRadian)
	{
		bInRange = false;
		for (ABattery* Obj : Objs)
		{				
			Obj->ActiveState(false, nullptr);
		}
		
		for (TSharedPtr<QuadTreeNode>& Node: Child_Node)
		{
			if (Node.IsValid())
			{
				Node->TraceObjectOutRange(InCenter, InRadian);
			}			
		}
	}

	// 更新状态
	void UpdateState()
	{
		DrawBound(1 / UKismetSystemLibrary::GetFrameCount()); //根据帧数绘制

		if (!bIsLeaf)
		{
			//如果不是叶子节点，则递归到子树下去，如果子树为空，则回收该节点
			for (TSharedPtr<QuadTreeNode>& Node : Child_Node)
			{
				if (Node.IsValid())
				{
					Node->UpdateState();
					if (Node->IsNotUsed())
					{
						Node.Reset();
						Node = nullptr;	
					}					
				}
			}

			int32 count = 4;
			for (TSharedPtr<QuadTreeNode>& Node : Child_Node)
			{
				if (!Node.IsValid())
				{
					count--;
				}
			}
			
			if (count == 0)
			{
				bIsLeaf = true;
			}
			else
			{
				return;
			}
		}
		
		if (bIsLeaf && Objs.Num()>0)
		{
			//如果叶子节点，更新物体是否在区域内；不在区域则移出，并重新插入
			int32 i = 0;
			while (i < Objs.Num())
			{
				if (!InterSection(Objs[i]->GetActorLocation()))
				{	
					ABattery* Battery = Objs[i];
					Objs.Swap(i, Objs.Num() - 1);
					Objs.Pop();
					Root->InsertObj(Battery);
					continue;
				}
				i++;
			}
		}
	}
};
