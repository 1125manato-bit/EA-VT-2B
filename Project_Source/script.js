/**
 * VT-2B BLACK - EMU AUDIO Plugin UI
 * 背景画像版 - インタラクティブノブコントロール
 */

class KnobController {
    constructor(element, options = {}) {
        this.element = element;
        this.valueDisplay = document.getElementById(options.valueId);

        this.min = options.min ?? 0;
        this.max = options.max ?? 100;
        this.value = options.initial ?? 0;
        this.step = options.step ?? 0.1;
        this.unit = options.unit ?? '';
        this.decimals = options.decimals ?? 1;
        this.defaultValue = options.default ?? this.value;

        this.isDragging = false;
        this.startY = 0;
        this.startValue = 0;

        this.init();
    }

    init() {
        this.updateDisplay();
        this.bindEvents();
    }

    bindEvents() {
        this.element.addEventListener('mousedown', (e) => this.onMouseDown(e));
        document.addEventListener('mousemove', (e) => this.onMouseMove(e));
        document.addEventListener('mouseup', () => this.onMouseUp());

        // タッチサポート
        this.element.addEventListener('touchstart', (e) => this.onTouchStart(e), { passive: false });
        document.addEventListener('touchmove', (e) => this.onTouchMove(e), { passive: false });
        document.addEventListener('touchend', () => this.onMouseUp());

        // ホイールサポート
        this.element.addEventListener('wheel', (e) => this.onWheel(e), { passive: false });

        // ダブルクリックでリセット
        this.element.addEventListener('dblclick', () => this.reset());
    }

    onMouseDown(e) {
        e.preventDefault();
        this.isDragging = true;
        this.startY = e.clientY;
        this.startValue = this.value;
        this.element.classList.add('dragging');
        document.body.style.cursor = 'ns-resize';
    }

    onTouchStart(e) {
        e.preventDefault();
        this.isDragging = true;
        this.startY = e.touches[0].clientY;
        this.startValue = this.value;
        this.element.classList.add('dragging');
    }

    onMouseMove(e) {
        if (!this.isDragging) return;

        const deltaY = this.startY - e.clientY;
        const sensitivity = e.shiftKey ? 0.1 : 0.5;
        const range = this.max - this.min;
        const delta = (deltaY * sensitivity * range) / 150;

        this.setValue(this.startValue + delta);
    }

    onTouchMove(e) {
        if (!this.isDragging) return;
        e.preventDefault();

        const deltaY = this.startY - e.touches[0].clientY;
        const sensitivity = 0.5;
        const range = this.max - this.min;
        const delta = (deltaY * sensitivity * range) / 150;

        this.setValue(this.startValue + delta);
    }

    onMouseUp() {
        if (this.isDragging) {
            this.isDragging = false;
            this.element.classList.remove('dragging');
            document.body.style.cursor = '';
        }
    }

    onWheel(e) {
        e.preventDefault();
        const delta = e.deltaY < 0 ? this.step * 2 : -this.step * 2;
        this.setValue(this.value + delta);
    }

    setValue(newValue) {
        const oldValue = this.value;
        this.value = Math.max(this.min, Math.min(this.max, newValue));

        // ステップに丸める
        this.value = Math.round(this.value / this.step) * this.step;

        if (oldValue !== this.value) {
            this.updateDisplay();
            this.element.setAttribute('data-value', this.value);

            // カスタムイベント発火
            this.element.dispatchEvent(new CustomEvent('knobchange', {
                detail: { value: this.value }
            }));
        }
    }

    reset() {
        this.setValue(this.defaultValue);
    }

    updateDisplay() {
        if (this.valueDisplay) {
            const formatted = this.value.toFixed(this.decimals);
            this.valueDisplay.textContent = formatted + this.unit;
        }
    }
}

// プリセット管理
class PresetManager {
    constructor(driveKnob, mixKnob) {
        this.driveKnob = driveKnob;
        this.mixKnob = mixKnob;
        this.presetCards = document.querySelectorAll('.preset-card');

        this.bindEvents();
    }

    bindEvents() {
        this.presetCards.forEach(card => {
            card.addEventListener('click', () => {
                const drive = parseFloat(card.dataset.drive);
                const mix = parseFloat(card.dataset.mix);

                this.driveKnob.setValue(drive);
                this.mixKnob.setValue(mix);

                // アクティブ表示
                this.presetCards.forEach(c => c.classList.remove('active'));
                card.classList.add('active');

                // プリセット名更新
                const presetName = document.querySelector('.preset-name');
                if (presetName) {
                    presetName.textContent = card.querySelector('.preset-title').textContent;
                }
            });
        });
    }
}

// 初期化
document.addEventListener('DOMContentLoaded', () => {
    // Driveノブ
    const driveKnob = new KnobController(document.getElementById('driveKnob'), {
        valueId: 'driveValue',
        min: 0,
        max: 100,
        initial: 0,
        default: 0,
        step: 1,
        decimals: 0
    });

    // Mixノブ
    const mixKnob = new KnobController(document.getElementById('mixKnob'), {
        valueId: 'mixValue',
        min: 0,
        max: 100,
        initial: 100,
        default: 100,
        step: 1,
        unit: '%',
        decimals: 0
    });

    // プリセットマネージャー
    new PresetManager(driveKnob, mixKnob);

    // キーボードショートカット
    document.addEventListener('keydown', (e) => {
        // Cmd/Ctrl + R でリセット
        if ((e.metaKey || e.ctrlKey) && e.key === 'r') {
            e.preventDefault();
            driveKnob.reset();
            mixKnob.reset();
        }
    });
});
