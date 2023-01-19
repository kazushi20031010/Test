#pragma once

class Game;
class Collision;

class Player : public IGameObject
{
public:
	//�v���C���[�̃X�e�[�g�B
	enum EnPlayerState {
		enPlayerState_Idle,					//�ҋ@�B
		enPlayerState_Walk,					//�����B
		enPlayerState_Attack,				//�U���B
		enPlayerState_Rolling,				//���
		enPlayerState_Damage,		//�_���\�W�󂯂��B
		enPlayerState_Down,					//HP��0�B
		enPlayerState_Clear					//�N���A�[�B
	};
public:
	Player();
	~Player();
	bool Start();
	void Update();
	void Render(RenderContext& rc);
	/// <returns>���W�B</returns>
	const Vector3& GetPosition() const
	{
		return m_position;
	}
	/// ��������(�X�e�[�g)���ǂ������擾�B
	bool IsEnableMove() const
	{
		return m_playerState != enPlayerState_Attack &&
			m_playerState != enPlayerState_Damage &&
			m_playerState != enPlayerState_Down &&
			m_playerState != enPlayerState_Clear;
	}
	//HP��ݒ肷��
	void SetHP(const int hp)
	{
		m_hp = hp;
	}
	//�ړ�����
	void Move();
	/// ��]�����B
	void Rotation();
	//�U������
	void Attack();
	/// �G�̍U���Ƃ̓����蔻�菈���B
	void Collision();
	/// �U�������ۂ̃G�t�F�N�g���쐬����B
	/// </summary>
	void MakeSlashingEffect();
	//�X�e�[�g�Ǘ��B
	void ManageState();
	/// ���ʂ̃X�e�[�g�J�ڏ����B
	void ProcessCommonStateTransition();
	/// �ҋ@�X�e�[�g�̑J�ڏ����B
	void ProcessIdleStateTransition();
	//�����X�e�[�g�̑J�ڏ����B
	void ProcessWalkStateTransition();
	/// �U���X�e�[�g�̑J�ڏ����B
	void ProcessAttackStateTransition();
	// ����X�e�[�g�̑J�ڏ����B
	void ProcessRollingStateTransition();
	// �_���[�W�X�e�[�g�̑J�ڏ����B
	void ProcessDamageStateTransition();
	// �_�E���X�e�[�g�̑J�ڏ����B
	void ProcessDownStateTransition();
	/// �N���A�X�e�[�g�̑J�ڏ����B
	/// </summary>
	void ProcessClearStateTransition();
	/// �U���̍ۂ̓����蔻��p�̃R���W�������쐬����B
	void MakeAttackCollision();
	//�A�j���[�V�����̍Đ��B
	void PlayAnimation();
	/// <param name="eventName">�A�j���[�V�����C�x���g�̃L�[�̖��O�B</param>
	void OnAnimationEvent(const wchar_t* clipName, const wchar_t* eventName);
	//////////////////////////////////////
	// �����o�ϐ��B
	//////////////////////////////////////
	/// �U�����̏����B
	/// </summary>
	

	enum EnAnimationClip {
		enAnimationClip_idle,	//�ҋ@�A�j���[�V�����B
		enAnimationClip_walk,	//�����A�j���[�V�����B
		enAnimationClip_attack, //�U���A�j���[�V�����B
		enAnimationClip_Rolling,//����A�j���[�V����
		enAnimationClip_Damage, //�_���[�W�A�j���[�V
		enAnimationClip_Down,	//�_�E���A�j���[�V����
		enAnimationClip_Clear,	//�N���A�A�j���[�V����
		enAnimationClip_Num,	//�A�j���[�V�����N���b�v�̐��B
	};
	ModelRender m_modelRender;									//���f�������_�\�B
	Vector3 m_position;											//���W�B
	Vector3 m_moveSpeed;										//�ړ����x�B
	Quaternion				m_rotation;							//�N�H�[�^�j�I���B
	Vector3 m_forward = Vector3::AxisZ;							//�v���C���[�̐��ʃx�N�g���B
	CharacterController m_charaCon;								//�L�����N�^�[�R���g���[���[�B
	EnPlayerState m_playerState =enPlayerState_Idle;			//�v���C���[�̃X�e�[�g(���)��\���ϐ��B
	bool					m_isUnderAttack = false;			//�U�����Ȃ�true�B
	int		m_swordBoneId =-1;									//���ɐݒ肵���{�[����ID�B
	int m_hp=10;												    //HP
	AnimationClip m_animClips[enAnimationClip_Num];				//�A�j���[�V�����N���b�v�B
	Game* m_game = nullptr;										//�Q�[���B
};