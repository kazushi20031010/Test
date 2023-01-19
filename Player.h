#pragma once

class Game;
class Collision;

class Player : public IGameObject
{
public:
	//プレイヤーのステート。
	enum EnPlayerState {
		enPlayerState_Idle,					//待機。
		enPlayerState_Walk,					//歩き。
		enPlayerState_Attack,				//攻撃。
		enPlayerState_Rolling,				//回避
		enPlayerState_Damage,		//ダメ―ジ受けた。
		enPlayerState_Down,					//HPが0。
		enPlayerState_Clear					//クリアー。
	};
public:
	Player();
	~Player();
	bool Start();
	void Update();
	void Render(RenderContext& rc);
	/// <returns>座標。</returns>
	const Vector3& GetPosition() const
	{
		return m_position;
	}
	/// 動ける状態(ステート)かどうかを取得。
	bool IsEnableMove() const
	{
		return m_playerState != enPlayerState_Attack &&
			m_playerState != enPlayerState_Damage &&
			m_playerState != enPlayerState_Down &&
			m_playerState != enPlayerState_Clear;
	}
	//HPを設定する
	void SetHP(const int hp)
	{
		m_hp = hp;
	}
	//移動処理
	void Move();
	/// 回転処理。
	void Rotation();
	//攻撃処理
	void Attack();
	/// 敵の攻撃との当たり判定処理。
	void Collision();
	/// 攻撃した際のエフェクトを作成する。
	/// </summary>
	void MakeSlashingEffect();
	//ステート管理。
	void ManageState();
	/// 共通のステート遷移処理。
	void ProcessCommonStateTransition();
	/// 待機ステートの遷移処理。
	void ProcessIdleStateTransition();
	//歩きステートの遷移処理。
	void ProcessWalkStateTransition();
	/// 攻撃ステートの遷移処理。
	void ProcessAttackStateTransition();
	// 回避ステートの遷移処理。
	void ProcessRollingStateTransition();
	// ダメージステートの遷移処理。
	void ProcessDamageStateTransition();
	// ダウンステートの遷移処理。
	void ProcessDownStateTransition();
	/// クリアステートの遷移処理。
	/// </summary>
	void ProcessClearStateTransition();
	/// 攻撃の際の当たり判定用のコリジョンを作成する。
	void MakeAttackCollision();
	//アニメーションの再生。
	void PlayAnimation();
	/// <param name="eventName">アニメーションイベントのキーの名前。</param>
	void OnAnimationEvent(const wchar_t* clipName, const wchar_t* eventName);
	//////////////////////////////////////
	// メンバ変数。
	//////////////////////////////////////
	/// 攻撃中の処理。
	/// </summary>
	

	enum EnAnimationClip {
		enAnimationClip_idle,	//待機アニメーション。
		enAnimationClip_walk,	//歩きアニメーション。
		enAnimationClip_attack, //攻撃アニメーション。
		enAnimationClip_Rolling,//回避アニメーション
		enAnimationClip_Damage, //ダメージアニメーシ
		enAnimationClip_Down,	//ダウンアニメーション
		enAnimationClip_Clear,	//クリアアニメーション
		enAnimationClip_Num,	//アニメーションクリップの数。
	};
	ModelRender m_modelRender;									//モデルレンダ―。
	Vector3 m_position;											//座標。
	Vector3 m_moveSpeed;										//移動速度。
	Quaternion				m_rotation;							//クォータニオン。
	Vector3 m_forward = Vector3::AxisZ;							//プレイヤーの正面ベクトル。
	CharacterController m_charaCon;								//キャラクターコントローラー。
	EnPlayerState m_playerState =enPlayerState_Idle;			//プレイヤーのステート(状態)を表す変数。
	bool					m_isUnderAttack = false;			//攻撃中ならtrue。
	int		m_swordBoneId =-1;									//剣に設定したボーンのID。
	int m_hp=10;												    //HP
	AnimationClip m_animClips[enAnimationClip_Num];				//アニメーションクリップ。
	Game* m_game = nullptr;										//ゲーム。
};