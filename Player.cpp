#include "stdafx.h"
#include "Player.h"

#include "Game.h"
#include "collision/CollisionObject.h"
#include "graphics/effect/EffectEmitter.h"
#include "sound/SoundEngine.h"
#include "sound/SoundSource.h"

Player::Player()
{
	
}

Player::~Player()
{

}

bool Player::Start()
{
	//キャラクターコントローラーを初期化。
	m_charaCon.Init(
		20.0f,			//半径。
		50.0f,			//高さ。
		m_position		//座標。
	);
	//アニメーションクリップのロード。
	m_animClips[enAnimationClip_idle].Load("Assets/animData/Player_idle.tka");
	m_animClips[enAnimationClip_walk].Load("Assets/animData/Player_walk.tka");
	m_animClips[enAnimationClip_attack].Load("Assets/animData/Player_attack.tka");
	m_animClips[enAnimationClip_Rolling].Load("Assets/animData/Player_Rolling.tka");
	m_animClips[enAnimationClip_Damage].Load("Assets/animData/Player_Damage.tka");
	m_animClips[enAnimationClip_Down].Load("Assets/animData/Player_Down.tka");
	m_animClips[enAnimationClip_Clear].Load("Assets/animData/Player_Clear.tka");
	//ループフラグを設定する。<-走りアニメーションはループフラグを設定していないので
	//ワンショット再生で停止する。
	m_animClips[enAnimationClip_idle].SetLoopFlag(true);
	m_animClips[enAnimationClip_walk].SetLoopFlag(true);
	m_animClips[enAnimationClip_attack].SetLoopFlag(false);
	m_animClips[enAnimationClip_Rolling].SetLoopFlag(false);
	m_animClips[enAnimationClip_Damage].SetLoopFlag(false);
	m_animClips[enAnimationClip_Down].SetLoopFlag(false);
	m_animClips[enAnimationClip_Clear].SetLoopFlag(false);
	//モデルを読み込む。
	m_modelRender.Init("Assets/modelData/Player.tkm", m_animClips, enAnimationClip_Num);

	//剣に設定したボーンのIDを取得する。
	m_swordBoneId = m_modelRender.FindBoneID(L"mixamorig:Sword_joint");
	//アニメーションイベント用の関数を設定する。
	m_modelRender.AddAnimationEvent([&](const wchar_t* clipName, const wchar_t* eventName) {
		OnAnimationEvent(clipName, eventName);
		});

	//各エフェクトをロードする。
	EffectEngine::GetInstance()->ResistEffect(1, u"Assets/effect/efk/enemy_slash_01.efk");

	//音を読み込む。
	g_soundEngine->ResistWaveFileBank(6, "Assets/sound/07_cool_attack.wav");
	g_soundEngine->ResistWaveFileBank(7, "Assets/sound/20_cool_avoid.wav");
	g_soundEngine->ResistWaveFileBank(8, "Assets/sound/15_cool_damage.wav");
	g_soundEngine->ResistWaveFileBank(9, "Assets/sound/23_cool_down.wav");

	m_game = FindGO<Game>("game");
	return true;
}

void Player::Update()
{
	//移動処理
	Move();

	//当たり判定。
	Collision();
	//攻撃処理
	Attack();
	//アニメーションの再生。
	PlayAnimation();
	//ステート管理
	ManageState();
	
	//モデルの更新。
	m_modelRender.Update();
}

void Player::Move()
{
	//移動できない状態であれば、移動処理はしない。
	if (IsEnableMove() == false)
	{
		return;
	}
	m_moveSpeed.x = 0.0f;
	m_moveSpeed.z = 0.0f;
	

	//このフレームの移動量を求める。
	//左スティックの入力量を受け取る。
	float lStick_y = g_pad[0]->GetLStickXF();
	float lStick_x = g_pad[0]->GetLStickYF();
	//カメラの前方方向と右方向を取得。
	Vector3 cameraForward = g_camera3D->GetForward();
	Vector3 cameraRight = g_camera3D->GetRight();
	//XZ平面での前方方向、右方向に変換する。
	cameraForward.y = 0.0f;
	cameraForward.Normalize();
	cameraRight.y = 0.0f;
	cameraRight.Normalize();
	//XZ成分の移動速度をクリア。
	m_moveSpeed += cameraForward * lStick_x * 250.0f;	//奥方向への移動速度を加算。
	m_moveSpeed += cameraRight * lStick_y * 250.0f;		//右方向への移動速度を加算。
	/*if (g_pad[0]->IsTrigger(enButtonA) //Aボタンが押されたら
		&& m_charaCon.IsOnGround()  //かつ、地面に居たら
		) {
		//ジャンプする。
		m_moveSpeed.y = 400.0f;	//上方向に速度を設定して、
	}*/
	/*if (g_pad[0]->IsTrigger(enButtonA)) {
		//ステートを回避にする
		m_playerState = enPlayerState_Rolling;
	}*/

		m_moveSpeed.y -= 980.0f * g_gameTime->GetFrameDeltaTime();

	//キャラクターコントローラーを使用して、座標を更新。
	m_position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());
	if (m_charaCon.IsOnGround()) {
		//地面についた。
		m_moveSpeed.y = 0.0f;
	}
	Vector3 modelPosition = m_position;
	//座標を設定。
	m_modelRender.SetPosition(m_position);
}

void Player::Rotation()
{
	if (fabsf(m_moveSpeed.x) < 0.001f
		&& fabsf(m_moveSpeed.z) < 0.001f) {
		//m_moveSpeed.xとm_moveSpeed.zの絶対値がともに0.001以下ということは
		//このフレームではキャラは移動していないので旋回する必要はない。
		return;
	}
	//atan2はtanθの値を角度(ラジアン単位)に変換してくれる関数。
	//m_moveSpeed.x / m_moveSpeed.zの結果はtanθになる。
	//atan2を使用して、角度を求めている。
	//これが回転角度になる。
	float angle = atan2(-m_moveSpeed.x, m_moveSpeed.z);
	//atanが返してくる角度はラジアン単位なので
	//SetRotationDegではなくSetRotationを使用する。
	m_rotation.SetRotationY(-angle);

	//回転を設定する。
	m_modelRender.SetRotation(m_rotation);

	//プレイヤーの正面ベクトルを計算する。
	m_forward = Vector3::AxisZ;
	m_rotation.Apply(m_forward);
}

void Player::Attack()
{
	//攻撃中でないなら、処理をしない。
	if (m_playerState != enPlayerState_Attack)
	{
		return;
	}
	//攻撃判定中であれば。
	if (m_isUnderAttack == true)
	{
		//攻撃用のコリジョンを作成する。
		MakeAttackCollision();
	}
}

void Player::Collision()
{
	if (m_playerState == enPlayerState_Damage||
		m_playerState==enPlayerState_Down||
		m_playerState==enPlayerState_Rolling)
	{
		return;
	}
	

	{
		const auto& collisions = g_collisionObjectManager->FindCollisionObjects("Enemy_attack");
		//配列をfor文で回す
		for (auto collision : collisions)
		{
			if (collision->IsHit(m_charaCon))
			{
				//HPを減らす
				m_hp -= 1;
				if (m_hp == 0)
				{
					m_playerState = enPlayerState_Down;
					SoundSource* se = NewGO<SoundSource>(0);
					se->Init(9);
					se->Play(false);
					se->SetVolume(0.4f);
				}
				else {
					m_playerState = enPlayerState_Damage;
					//効果音を再生する。
					SoundSource* se = NewGO<SoundSource>(0);
					se->Init(8);
					se->Play(false);
					se->SetVolume(0.4f);
				}
				return;
			}
		}
	}
	{
		//敵の撃った弾のコリジョンの配列を取得する。
		const auto& collisions = g_collisionObjectManager->FindCollisionObjects("enemy_ball");
		//配列をfor文で回す。
		for (auto collision : collisions)
		{
			//コリジョンとキャラコンが衝突したら。
			if (collision->IsHit(m_charaCon))
			{
				//HPを減らす。
				m_hp -= 1;
				//HPが0になったら。
				if (m_hp == 0)
				{
					//ダウンさせる。
					m_playerState = enPlayerState_Down;
					SoundSource* se = NewGO<SoundSource>(0);
					se->Init(9);
					se->Play(false);
					se->SetVolume(0.4f);
				}
				//HPが0ではなかったら。
				else {
					//被ダメージリアクションをさせる。
					m_playerState = enPlayerState_Damage;
					//効果音を再生する。
					SoundSource* se = NewGO<SoundSource>(0);
					se->Init(8);
					se->Play(false);
					se->SetVolume(0.4f);
				}
				return;
			}
		}
	}
	
}

void Player::MakeAttackCollision()
{
	//コリジョンオブジェクトを作成する。
	auto collisionObject = NewGO<CollisionObject>(0);
	
	Vector3 collisionPosition = m_position;
	//座標をプレイヤーの少し前に設定する。
	collisionPosition += m_forward * 50.0f;
	//ボックス状のコリジョンを作成する。
	collisionObject->CreateBox(collisionPosition, //座標。
		Quaternion::Identity, //回転。
		Vector3(100.0f, 15.0f, 15.0f) //大きさ。
	);
	collisionObject->SetName("Player_attack");

	//「Sword」ボーンのワールド行列を取得する。
	Matrix matrix = m_modelRender.GetBone(m_swordBoneId)->GetWorldMatrix();
	//「Sword」ボーンのワールド行列をコリジョンに適用する。
	collisionObject->SetWorldMatrix(matrix);
}

void Player::MakeSlashingEffect()
{
	//攻撃エフェクトを発生させる。
	EffectEmitter* effectEmitter = NewGO<EffectEmitter>(0);
	effectEmitter->Init(1);
	effectEmitter->SetScale(Vector3::One * 5.0f);
	Vector3 effectPosition = m_position;
	//座標を少し上にする。
	effectPosition.y += 50.0f;
	effectEmitter->SetPosition(effectPosition);
	Quaternion rotation;
	rotation = m_rotation;
	//回転させる。
	rotation.AddRotationDegY(360.0f);
	rotation.AddRotationDegZ(180.0f);
	effectEmitter->SetRotation(rotation);
	//エフェクトを再生する。
	effectEmitter->Play();
}

void Player::ProcessCommonStateTransition()
{
	//敵を全滅させたら。
	if (m_game->IsWannihilationEnemy())
	{
		//クリアステートに移行する。
		m_playerState = enPlayerState_Clear;
		return;
	}
	
	//xかzの移動速度があったら(スティックの入力があったら)。
	if (fabsf(m_moveSpeed.x) >= 0.001f || fabsf(m_moveSpeed.z) >= 0.001f)
	{
		//ステートを歩きにする。
		m_playerState = enPlayerState_Walk;
		return;
	}

	//Bボタンが押されたら。
	if (g_pad[0]->IsTrigger(enButtonB)) {
		//ステートを攻撃にする。
		m_playerState = enPlayerState_Attack;
		//フラグをfalseにする。
		m_isUnderAttack = false;
		return;
	}

	//Aボタンが押されたら
	if (g_pad[0]->IsTrigger(enButtonA)) {
		//ステートを回避にする
		m_playerState = enPlayerState_Rolling;
		//効果音を再生する。
		SoundSource* se = NewGO<SoundSource>(0);
		se->Init(7);
		se->Play(false);
		se->SetVolume(0.4f);
		return;
	}

	//xとzの移動速度が無かったら(スティックの入力が無かったら)。
	else
	{
		//ステートを待機にする。
		m_playerState = enPlayerState_Idle;
		return;
	}
}

void Player::ProcessIdleStateTransition()
{
	//ステートを遷移する。
	ProcessCommonStateTransition();
}

void Player::ProcessWalkStateTransition()
{
	//ステートを遷移する。
	ProcessCommonStateTransition();
}

void Player::ProcessAttackStateTransition()
{
	//攻撃アニメーションの再生が終わったら。
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//ステートを遷移する。
		ProcessCommonStateTransition();
	}
}

void Player::ProcessRollingStateTransition()
{
	//回避アニメーションの再生が終わったら
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//ステートを遷移する。
		ProcessCommonStateTransition();
	}

}

void Player::ProcessDamageStateTransition()
{
	//ダメージアニメーションの再生が終わったら
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		ProcessCommonStateTransition();
	}
}

void Player::ProcessDownStateTransition()
{
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//ゲームオーバーを通知する。
		m_game->NotifyGameOver();
	}
}

void Player::ProcessClearStateTransition()
{
	//勝利アニメーションの再生が終わったら。
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//ゲームクリアを通知する。
		m_game->NotifyGameClear();
	}
}

//ステート管理。
void Player::ManageState()
{
	switch (m_playerState)
	{
		//待機ステートの時。
	case enPlayerState_Idle:
		//待機ステートのステート遷移処理。
		ProcessIdleStateTransition();
		break;
		//歩きステートの時。
	case enPlayerState_Walk:
		//歩きステートのステート遷移処理。
		ProcessWalkStateTransition();
		break;
	case enPlayerState_Attack:
		//攻撃ステートのステート遷移処理。
		ProcessAttackStateTransition();
		break;
	case enPlayerState_Rolling:
		//回避ステートのステート遷移処理。
		ProcessRollingStateTransition();

		break;
	case enPlayerState_Damage:
		//ダメージステートの遷移処理。
		ProcessDamageStateTransition();
		break;
	case enPlayerState_Down:
		//ダウンステートの遷移処理。
		ProcessDownStateTransition();
		break;
		//クリアステートの時。
	case enPlayerState_Clear:
		//クリアステートのステート遷移処理。
		ProcessClearStateTransition();
		break;
	}
}

void Player::PlayAnimation()
{
	//switch文。
	switch (m_playerState) {
		//プレイヤーステートが待機だったら。
	case enPlayerState_Idle:
		//待機アニメーションを再生する。
		m_modelRender.PlayAnimation(enAnimationClip_idle);
		break;
		//プレイヤーステートが歩きだったら。
	case enPlayerState_Walk:
		//歩きアニメーションを再生する。
		m_modelRender.PlayAnimation(enAnimationClip_walk);
		break;
		//プレイヤーステートが攻撃だったら。
	case enPlayerState_Attack:
		//攻撃アニメーションを再生する。
		m_modelRender.PlayAnimation(enAnimationClip_attack);
		break;
		//プレイヤーステートが回避だったら。
	case enPlayerState_Rolling:
		//回避アニメーションを再生する
		m_modelRender.PlayAnimation(enAnimationClip_Rolling);

		break;
		//プレイヤーステートがダメージだったら。
	case enPlayerState_Damage:
		//ダメージアニメーションを再生する。
		m_modelRender.PlayAnimation(enAnimationClip_Damage);
		break;
		//プレイヤーステートがダウンだったら。
	case enPlayerState_Down:
		//ダウンアニメーションを再生する。
		m_modelRender.PlayAnimation(enAnimationClip_Down);
		break;
		//クリアステートの時。
	case enPlayerState_Clear:
		//クリアアニメーションを再生。
		m_modelRender.PlayAnimation(enAnimationClip_Clear, 0.1f);
		break;
	}
	
}

void Player::OnAnimationEvent(const wchar_t* clipName, const wchar_t* eventName)
{
	(void)clipName;
	//キーの名前が「attack_start」の時。
	if (wcscmp(eventName, L"attack_start") == 0) {
		//斬撃エフェクトを作成する。
		MakeSlashingEffect();
		//攻撃中にする。
		m_isUnderAttack = true;
		//効果音を再生する。
		SoundSource* se = NewGO<SoundSource>(0);
		se->Init(6);
		se->Play(false);
		se->SetVolume(0.4f);
	}
	//キーの名前が「attack_end」の時。
	else if (wcscmp(eventName, L"attack_end") == 0) {
		//攻撃を終わる。
		m_isUnderAttack = false;
	}
}
void Player::Render(RenderContext& rc)
{
	//モデルの描画。
	m_modelRender.Draw(rc);
}