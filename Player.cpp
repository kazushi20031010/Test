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
	//�L�����N�^�[�R���g���[���[���������B
	m_charaCon.Init(
		20.0f,			//���a�B
		50.0f,			//�����B
		m_position		//���W�B
	);
	//�A�j���[�V�����N���b�v�̃��[�h�B
	m_animClips[enAnimationClip_idle].Load("Assets/animData/Player_idle.tka");
	m_animClips[enAnimationClip_walk].Load("Assets/animData/Player_walk.tka");
	m_animClips[enAnimationClip_attack].Load("Assets/animData/Player_attack.tka");
	m_animClips[enAnimationClip_Rolling].Load("Assets/animData/Player_Rolling.tka");
	m_animClips[enAnimationClip_Damage].Load("Assets/animData/Player_Damage.tka");
	m_animClips[enAnimationClip_Down].Load("Assets/animData/Player_Down.tka");
	m_animClips[enAnimationClip_Clear].Load("Assets/animData/Player_Clear.tka");
	//���[�v�t���O��ݒ肷��B<-����A�j���[�V�����̓��[�v�t���O��ݒ肵�Ă��Ȃ��̂�
	//�����V���b�g�Đ��Œ�~����B
	m_animClips[enAnimationClip_idle].SetLoopFlag(true);
	m_animClips[enAnimationClip_walk].SetLoopFlag(true);
	m_animClips[enAnimationClip_attack].SetLoopFlag(false);
	m_animClips[enAnimationClip_Rolling].SetLoopFlag(false);
	m_animClips[enAnimationClip_Damage].SetLoopFlag(false);
	m_animClips[enAnimationClip_Down].SetLoopFlag(false);
	m_animClips[enAnimationClip_Clear].SetLoopFlag(false);
	//���f����ǂݍ��ށB
	m_modelRender.Init("Assets/modelData/Player.tkm", m_animClips, enAnimationClip_Num);

	//���ɐݒ肵���{�[����ID���擾����B
	m_swordBoneId = m_modelRender.FindBoneID(L"mixamorig:Sword_joint");
	//�A�j���[�V�����C�x���g�p�̊֐���ݒ肷��B
	m_modelRender.AddAnimationEvent([&](const wchar_t* clipName, const wchar_t* eventName) {
		OnAnimationEvent(clipName, eventName);
		});

	//�e�G�t�F�N�g�����[�h����B
	EffectEngine::GetInstance()->ResistEffect(1, u"Assets/effect/efk/enemy_slash_01.efk");

	//����ǂݍ��ށB
	g_soundEngine->ResistWaveFileBank(6, "Assets/sound/07_cool_attack.wav");
	g_soundEngine->ResistWaveFileBank(7, "Assets/sound/20_cool_avoid.wav");
	g_soundEngine->ResistWaveFileBank(8, "Assets/sound/15_cool_damage.wav");
	g_soundEngine->ResistWaveFileBank(9, "Assets/sound/23_cool_down.wav");

	m_game = FindGO<Game>("game");
	return true;
}

void Player::Update()
{
	//�ړ�����
	Move();

	//�����蔻��B
	Collision();
	//�U������
	Attack();
	//�A�j���[�V�����̍Đ��B
	PlayAnimation();
	//�X�e�[�g�Ǘ�
	ManageState();
	
	//���f���̍X�V�B
	m_modelRender.Update();
}

void Player::Move()
{
	//�ړ��ł��Ȃ���Ԃł���΁A�ړ������͂��Ȃ��B
	if (IsEnableMove() == false)
	{
		return;
	}
	m_moveSpeed.x = 0.0f;
	m_moveSpeed.z = 0.0f;
	

	//���̃t���[���̈ړ��ʂ����߂�B
	//���X�e�B�b�N�̓��͗ʂ��󂯎��B
	float lStick_y = g_pad[0]->GetLStickXF();
	float lStick_x = g_pad[0]->GetLStickYF();
	//�J�����̑O�������ƉE�������擾�B
	Vector3 cameraForward = g_camera3D->GetForward();
	Vector3 cameraRight = g_camera3D->GetRight();
	//XZ���ʂł̑O�������A�E�����ɕϊ�����B
	cameraForward.y = 0.0f;
	cameraForward.Normalize();
	cameraRight.y = 0.0f;
	cameraRight.Normalize();
	//XZ�����̈ړ����x���N���A�B
	m_moveSpeed += cameraForward * lStick_x * 250.0f;	//�������ւ̈ړ����x�����Z�B
	m_moveSpeed += cameraRight * lStick_y * 250.0f;		//�E�����ւ̈ړ����x�����Z�B
	/*if (g_pad[0]->IsTrigger(enButtonA) //A�{�^���������ꂽ��
		&& m_charaCon.IsOnGround()  //���A�n�ʂɋ�����
		) {
		//�W�����v����B
		m_moveSpeed.y = 400.0f;	//������ɑ��x��ݒ肵�āA
	}*/
	/*if (g_pad[0]->IsTrigger(enButtonA)) {
		//�X�e�[�g������ɂ���
		m_playerState = enPlayerState_Rolling;
	}*/

		m_moveSpeed.y -= 980.0f * g_gameTime->GetFrameDeltaTime();

	//�L�����N�^�[�R���g���[���[���g�p���āA���W���X�V�B
	m_position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());
	if (m_charaCon.IsOnGround()) {
		//�n�ʂɂ����B
		m_moveSpeed.y = 0.0f;
	}
	Vector3 modelPosition = m_position;
	//���W��ݒ�B
	m_modelRender.SetPosition(m_position);
}

void Player::Rotation()
{
	if (fabsf(m_moveSpeed.x) < 0.001f
		&& fabsf(m_moveSpeed.z) < 0.001f) {
		//m_moveSpeed.x��m_moveSpeed.z�̐�Βl���Ƃ���0.001�ȉ��Ƃ������Ƃ�
		//���̃t���[���ł̓L�����͈ړ����Ă��Ȃ��̂Ő��񂷂�K�v�͂Ȃ��B
		return;
	}
	//atan2��tan�Ƃ̒l���p�x(���W�A���P��)�ɕϊ����Ă����֐��B
	//m_moveSpeed.x / m_moveSpeed.z�̌��ʂ�tan�ƂɂȂ�B
	//atan2���g�p���āA�p�x�����߂Ă���B
	//���ꂪ��]�p�x�ɂȂ�B
	float angle = atan2(-m_moveSpeed.x, m_moveSpeed.z);
	//atan���Ԃ��Ă���p�x�̓��W�A���P�ʂȂ̂�
	//SetRotationDeg�ł͂Ȃ�SetRotation���g�p����B
	m_rotation.SetRotationY(-angle);

	//��]��ݒ肷��B
	m_modelRender.SetRotation(m_rotation);

	//�v���C���[�̐��ʃx�N�g�����v�Z����B
	m_forward = Vector3::AxisZ;
	m_rotation.Apply(m_forward);
}

void Player::Attack()
{
	//�U�����łȂ��Ȃ�A���������Ȃ��B
	if (m_playerState != enPlayerState_Attack)
	{
		return;
	}
	//�U�����蒆�ł���΁B
	if (m_isUnderAttack == true)
	{
		//�U���p�̃R���W�������쐬����B
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
		//�z���for���ŉ�
		for (auto collision : collisions)
		{
			if (collision->IsHit(m_charaCon))
			{
				//HP�����炷
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
					//���ʉ����Đ�����B
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
		//�G�̌������e�̃R���W�����̔z����擾����B
		const auto& collisions = g_collisionObjectManager->FindCollisionObjects("enemy_ball");
		//�z���for���ŉ񂷁B
		for (auto collision : collisions)
		{
			//�R���W�����ƃL�����R�����Փ˂�����B
			if (collision->IsHit(m_charaCon))
			{
				//HP�����炷�B
				m_hp -= 1;
				//HP��0�ɂȂ�����B
				if (m_hp == 0)
				{
					//�_�E��������B
					m_playerState = enPlayerState_Down;
					SoundSource* se = NewGO<SoundSource>(0);
					se->Init(9);
					se->Play(false);
					se->SetVolume(0.4f);
				}
				//HP��0�ł͂Ȃ�������B
				else {
					//��_���[�W���A�N�V������������B
					m_playerState = enPlayerState_Damage;
					//���ʉ����Đ�����B
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
	//�R���W�����I�u�W�F�N�g���쐬����B
	auto collisionObject = NewGO<CollisionObject>(0);
	
	Vector3 collisionPosition = m_position;
	//���W���v���C���[�̏����O�ɐݒ肷��B
	collisionPosition += m_forward * 50.0f;
	//�{�b�N�X��̃R���W�������쐬����B
	collisionObject->CreateBox(collisionPosition, //���W�B
		Quaternion::Identity, //��]�B
		Vector3(100.0f, 15.0f, 15.0f) //�傫���B
	);
	collisionObject->SetName("Player_attack");

	//�uSword�v�{�[���̃��[���h�s����擾����B
	Matrix matrix = m_modelRender.GetBone(m_swordBoneId)->GetWorldMatrix();
	//�uSword�v�{�[���̃��[���h�s����R���W�����ɓK�p����B
	collisionObject->SetWorldMatrix(matrix);
}

void Player::MakeSlashingEffect()
{
	//�U���G�t�F�N�g�𔭐�������B
	EffectEmitter* effectEmitter = NewGO<EffectEmitter>(0);
	effectEmitter->Init(1);
	effectEmitter->SetScale(Vector3::One * 5.0f);
	Vector3 effectPosition = m_position;
	//���W��������ɂ���B
	effectPosition.y += 50.0f;
	effectEmitter->SetPosition(effectPosition);
	Quaternion rotation;
	rotation = m_rotation;
	//��]������B
	rotation.AddRotationDegY(360.0f);
	rotation.AddRotationDegZ(180.0f);
	effectEmitter->SetRotation(rotation);
	//�G�t�F�N�g���Đ�����B
	effectEmitter->Play();
}

void Player::ProcessCommonStateTransition()
{
	//�G��S�ł�������B
	if (m_game->IsWannihilationEnemy())
	{
		//�N���A�X�e�[�g�Ɉڍs����B
		m_playerState = enPlayerState_Clear;
		return;
	}
	
	//x��z�̈ړ����x����������(�X�e�B�b�N�̓��͂���������)�B
	if (fabsf(m_moveSpeed.x) >= 0.001f || fabsf(m_moveSpeed.z) >= 0.001f)
	{
		//�X�e�[�g������ɂ���B
		m_playerState = enPlayerState_Walk;
		return;
	}

	//B�{�^���������ꂽ��B
	if (g_pad[0]->IsTrigger(enButtonB)) {
		//�X�e�[�g���U���ɂ���B
		m_playerState = enPlayerState_Attack;
		//�t���O��false�ɂ���B
		m_isUnderAttack = false;
		return;
	}

	//A�{�^���������ꂽ��
	if (g_pad[0]->IsTrigger(enButtonA)) {
		//�X�e�[�g������ɂ���
		m_playerState = enPlayerState_Rolling;
		//���ʉ����Đ�����B
		SoundSource* se = NewGO<SoundSource>(0);
		se->Init(7);
		se->Play(false);
		se->SetVolume(0.4f);
		return;
	}

	//x��z�̈ړ����x������������(�X�e�B�b�N�̓��͂�����������)�B
	else
	{
		//�X�e�[�g��ҋ@�ɂ���B
		m_playerState = enPlayerState_Idle;
		return;
	}
}

void Player::ProcessIdleStateTransition()
{
	//�X�e�[�g��J�ڂ���B
	ProcessCommonStateTransition();
}

void Player::ProcessWalkStateTransition()
{
	//�X�e�[�g��J�ڂ���B
	ProcessCommonStateTransition();
}

void Player::ProcessAttackStateTransition()
{
	//�U���A�j���[�V�����̍Đ����I�������B
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//�X�e�[�g��J�ڂ���B
		ProcessCommonStateTransition();
	}
}

void Player::ProcessRollingStateTransition()
{
	//����A�j���[�V�����̍Đ����I�������
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//�X�e�[�g��J�ڂ���B
		ProcessCommonStateTransition();
	}

}

void Player::ProcessDamageStateTransition()
{
	//�_���[�W�A�j���[�V�����̍Đ����I�������
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		ProcessCommonStateTransition();
	}
}

void Player::ProcessDownStateTransition()
{
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//�Q�[���I�[�o�[��ʒm����B
		m_game->NotifyGameOver();
	}
}

void Player::ProcessClearStateTransition()
{
	//�����A�j���[�V�����̍Đ����I�������B
	if (m_modelRender.IsPlayingAnimation() == false)
	{
		//�Q�[���N���A��ʒm����B
		m_game->NotifyGameClear();
	}
}

//�X�e�[�g�Ǘ��B
void Player::ManageState()
{
	switch (m_playerState)
	{
		//�ҋ@�X�e�[�g�̎��B
	case enPlayerState_Idle:
		//�ҋ@�X�e�[�g�̃X�e�[�g�J�ڏ����B
		ProcessIdleStateTransition();
		break;
		//�����X�e�[�g�̎��B
	case enPlayerState_Walk:
		//�����X�e�[�g�̃X�e�[�g�J�ڏ����B
		ProcessWalkStateTransition();
		break;
	case enPlayerState_Attack:
		//�U���X�e�[�g�̃X�e�[�g�J�ڏ����B
		ProcessAttackStateTransition();
		break;
	case enPlayerState_Rolling:
		//����X�e�[�g�̃X�e�[�g�J�ڏ����B
		ProcessRollingStateTransition();

		break;
	case enPlayerState_Damage:
		//�_���[�W�X�e�[�g�̑J�ڏ����B
		ProcessDamageStateTransition();
		break;
	case enPlayerState_Down:
		//�_�E���X�e�[�g�̑J�ڏ����B
		ProcessDownStateTransition();
		break;
		//�N���A�X�e�[�g�̎��B
	case enPlayerState_Clear:
		//�N���A�X�e�[�g�̃X�e�[�g�J�ڏ����B
		ProcessClearStateTransition();
		break;
	}
}

void Player::PlayAnimation()
{
	//switch���B
	switch (m_playerState) {
		//�v���C���[�X�e�[�g���ҋ@��������B
	case enPlayerState_Idle:
		//�ҋ@�A�j���[�V�������Đ�����B
		m_modelRender.PlayAnimation(enAnimationClip_idle);
		break;
		//�v���C���[�X�e�[�g��������������B
	case enPlayerState_Walk:
		//�����A�j���[�V�������Đ�����B
		m_modelRender.PlayAnimation(enAnimationClip_walk);
		break;
		//�v���C���[�X�e�[�g���U����������B
	case enPlayerState_Attack:
		//�U���A�j���[�V�������Đ�����B
		m_modelRender.PlayAnimation(enAnimationClip_attack);
		break;
		//�v���C���[�X�e�[�g�������������B
	case enPlayerState_Rolling:
		//����A�j���[�V�������Đ�����
		m_modelRender.PlayAnimation(enAnimationClip_Rolling);

		break;
		//�v���C���[�X�e�[�g���_���[�W��������B
	case enPlayerState_Damage:
		//�_���[�W�A�j���[�V�������Đ�����B
		m_modelRender.PlayAnimation(enAnimationClip_Damage);
		break;
		//�v���C���[�X�e�[�g���_�E����������B
	case enPlayerState_Down:
		//�_�E���A�j���[�V�������Đ�����B
		m_modelRender.PlayAnimation(enAnimationClip_Down);
		break;
		//�N���A�X�e�[�g�̎��B
	case enPlayerState_Clear:
		//�N���A�A�j���[�V�������Đ��B
		m_modelRender.PlayAnimation(enAnimationClip_Clear, 0.1f);
		break;
	}
	
}

void Player::OnAnimationEvent(const wchar_t* clipName, const wchar_t* eventName)
{
	(void)clipName;
	//�L�[�̖��O���uattack_start�v�̎��B
	if (wcscmp(eventName, L"attack_start") == 0) {
		//�a���G�t�F�N�g���쐬����B
		MakeSlashingEffect();
		//�U�����ɂ���B
		m_isUnderAttack = true;
		//���ʉ����Đ�����B
		SoundSource* se = NewGO<SoundSource>(0);
		se->Init(6);
		se->Play(false);
		se->SetVolume(0.4f);
	}
	//�L�[�̖��O���uattack_end�v�̎��B
	else if (wcscmp(eventName, L"attack_end") == 0) {
		//�U�����I���B
		m_isUnderAttack = false;
	}
}
void Player::Render(RenderContext& rc)
{
	//���f���̕`��B
	m_modelRender.Draw(rc);
}